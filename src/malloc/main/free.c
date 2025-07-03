/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 23:03:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Free PTR"

	static int free_ptr(t_arena *arena, void *ptr, t_heap *heap) {
		if (!arena || !ptr || !heap) return (0);

		// LARGE
		if (heap->type == LARGE) {
			if (GET_HEAD(ptr) == heap->ptr) {

				// Corruption
				if (!HAS_MAGIC(ptr)) {
					if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted (LARGE)\n", ptr);
					if (print_error())		aprintf(2, 0, "Memory corrupted\n");
					return (abort_now());
				}

				if (heap_destroy(heap)) arena->free_count++;
				return (0);
			}

			if (print_log(1))				aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in middle of chunk (LARGE))\n", ptr);
			if (print_error())				aprintf(2, 0, "free: Invalid pointer\n");
			return (abort_now());
		}

		// Double free
		if (HAS_POISON(ptr)) {
			if (print_log(1))				aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Double free (Poison)\n", ptr);
			if (print_error())				aprintf(2, 0, "free: Double free\n");
			return (abort_now());
		}

		// Top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK)) {
			if (print_log(1))				aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			if (print_error())				aprintf(2, 0, "free: Invalid pointer\n");
			return (abort_now());
		}

		// Middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) {
			if (print_log(1))				aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in middle of chunks (not allocated))\n", ptr);
			if (print_error())				aprintf(2, 0, "free: Invalid pointer\n");
			return (abort_now());
		}

		// Corruption
		if (!HAS_MAGIC(ptr)) {
			if (print_log(1))				aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted\n", ptr);
			if (print_error())				aprintf(2, 0, "Memory corrupted\n");
			return (abort_now());
		}

		SET_POISON(ptr);
		heap->free += GET_SIZE(chunk) + sizeof(t_chunk);

		// Add to bins
		next_chunk->size &= ~PREV_INUSE;
		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		SET_PREV_SIZE(next_chunk, GET_SIZE(chunk));
		if (chunk_size <= (size_t)g_manager.options.MXFAST) {
			link_chunk(chunk, chunk_size, FASTBIN, arena, heap);
		} else {
			chunk = coalescing_neighbours(chunk, arena, heap);
			if (!(chunk->size & TOP_CHUNK)) {
				link_chunk(chunk, chunk_size, UNSORTEDBIN, arena, heap);
			}
		}

		arena->free_count++;

		if (heap->free >= heap->size) {
			if (heap_can_removed(arena, heap)) {
				t_chunk *chunk = heap->ptr;
				bool cancel = false;
				while (!IS_TOPCHUNK(chunk)) {
					cancel = unlink_chunk(chunk, arena, heap);
					if (cancel) break;
					chunk = GET_NEXT(chunk);
				}
				if (!cancel) heap_destroy(heap);
			}
		}

		if (print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t   [FREE] Memory freed of size %d bytes\n", ptr, chunk_size);

		return (0);
	}
	
#pragma endregion

#pragma region "Free"

	__attribute__((visibility("default")))
	void free(void *ptr) {
		ensure_init();

		if (!ptr) return;

		// Not aligned
		if ((uintptr_t)ptr % ALIGNMENT) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
			if (print_error())		aprintf(2, 0, "free: Invalid pointer\n");
			abort_now(); return ;
		}

		// alloc zero
		if (check_digit(ptr, ZERO_MALLOC_BASE)) {
			mutex(&g_manager.mutex, MTX_LOCK);

				if (ptr > ZERO_MALLOC_BASE && ptr < (void *)((char *)ZERO_MALLOC_BASE + (g_manager.alloc_zero_counter * ALIGNMENT))) {
					if (print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t   [FREE] Memory freed of size 0 bytes\n", ptr);
					mutex(&tcache->mutex, MTX_LOCK);

						tcache->free_count--;

					mutex(&tcache->mutex, MTX_UNLOCK);
				}
			
			mutex(&g_manager.mutex, MTX_UNLOCK);
			return ;
		}

		t_arena	*arena = &g_manager.arena;
		t_heap	*heap = NULL;
		t_heap	*inactive = NULL;

		mutex(&g_manager.mutex, MTX_LOCK);

			while (arena) {
				mutex(&arena->mutex, MTX_LOCK);

					if ((heap = heap_find(arena, ptr))) {
						if (heap->active) {
							free_ptr(arena, ptr, heap);
							mutex(&arena->mutex, MTX_UNLOCK);
							mutex(&g_manager.mutex, MTX_UNLOCK);
							return ;
						} else inactive = heap;
					}

				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);

		// stress-ng --malloc 4 --malloc-bytes 1G --verify --timeout 10s
		// git clone https://github.com/redis/redis
		// ffmpeg -f lavfi -i testsrc=duration=10:size=1920x1080 -c:v libx264 test.mp4

		// Heap freed
		if (inactive) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (posible free when heap is unmamped)\n", ptr);
			if (print_error())		aprintf(2, 0, "free: Invalid pointer\n");
			abort_now(); return ;
		}
	}

#pragma endregion

#pragma region "Information"

	// Frees a block of memory previously allocated.
	//
	//   void free(void *ptr);
	//
	//   ptr   – pointer returned by malloc/calloc/realloc.
	//
	//   • On success: the memory block pointed to by ptr is deallocated.
	//   • On failure: undefined behavior.
	//
	// Notes:
	//   • ptr can be NULL. In that case, free() does nothing.
	//   • After freeing, ptr becomes invalid. Do not access the memory after calling free().
	//   • It is the user's responsibility to ensure that ptr points to a valid allocated block.

#pragma endregion
