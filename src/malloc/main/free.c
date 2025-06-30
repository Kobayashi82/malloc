/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 11:08:39 by vzurera-         ###   ########.fr       */
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
					if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Corrupted memory (LARGE)\n", ptr);
					else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Corrupted memory\n");
					return (abort_now());
				}

				if (heap_destroy(heap)) arena->free_count++;
				return (0);
			}

			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in middle of chunk (LARGE))\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			return (abort_now());
		}

		// Double free
		if (HAS_POISON(ptr)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Double free (Poison)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Double free\n");
			return (abort_now());
		}

		// Top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			return (abort_now());
		}

		// Middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in middle of chunks (not allocated))\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			return (abort_now());
		}

		// Corruption
		if (!HAS_MAGIC(ptr)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Corrupted memory\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Corrupted memory\n");
			return (abort_now());
		}

		SET_POISON(ptr);
		heap->free += GET_SIZE(chunk) + sizeof(t_chunk);

		// Add to bins
		next_chunk->size &= ~PREV_INUSE;
		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		SET_PREV_SIZE(next_chunk, GET_SIZE(chunk));
		if (chunk_size <= (size_t)g_manager.options.MXFAST) {
			if (g_manager.options.DEBUG)		aprintf(g_manager.options.fd_out, "%p\t [SYSTEM] Chunk added to FastBin\n", chunk);
			link_chunk(chunk, chunk_size, FASTBIN, arena);
		} else {
			if (g_manager.options.DEBUG)		aprintf(g_manager.options.fd_out, "%p\t [SYSTEM] Coalescing adjacent chunks\n", chunk);
			chunk = coalescing(chunk, arena, heap);
			if (!(chunk->size & TOP_CHUNK)) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [SYSTEM] Chunk added to UnsortedBin\n", chunk);
				link_chunk(chunk, chunk_size, UNSORTEDBIN, arena);
			}
		}

		arena->free_count++;

		if (heap->free == heap->size) {
			// Mark for elimination
		}

		if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t   [FREE] Memory freed of size %d bytes\n", ptr, chunk_size);

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
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			abort_now(); return ;
		}

		// alloc zero
		if (check_digit(ptr, ZERO_MALLOC_BASE)) {
			mutex(&g_manager.mutex, MTX_LOCK);

				if (ptr > ZERO_MALLOC_BASE && ptr < (void *)((char *)ZERO_MALLOC_BASE + (g_manager.zero_malloc_counter * ALIGNMENT))) {
					if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t   [FREE] Memory freed of size 0 bytes\n", ptr);
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

		// Heap freed
		if (inactive && (inactive->type != LARGE || GET_HEAD(ptr) == inactive->ptr)) {
			if		(g_manager.options.DEBUG)				aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Double free (inactive)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)	aprintf(2, "Double free\n");
			abort_now(); return ;
		}

		// Delegate
		if (!inactive && !heap) native_free(ptr);
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
