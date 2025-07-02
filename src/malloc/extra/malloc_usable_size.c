/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc_usable_size.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 11:48:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 09:44:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Usable PTR"

	static size_t usable_ptr(void *ptr, t_heap *heap) {
		if (!ptr || !heap) return (0);

		// Not aligned
		if (!IS_ALIGNED(ptr)) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
			if (print_error())			aprintf(2, 0, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// Heap freed
		if (!heap->active) {
			if (heap->type == LARGE && GET_PTR(heap->ptr) == ptr) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (freed)\n", ptr);
				if (print_error())		aprintf(2, 0, "Invalid pointer\n");
			} else {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				if (print_error())		aprintf(2, 0, "Invalid pointer\n");
			}
			abort_now();
			return (0);
		}

		// LARGE
		if (heap->type == LARGE) {
				if (GET_HEAD(ptr) != heap->ptr) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				if (print_error())		aprintf(2, 0, "Invalid pointer\n");
				abort_now();
				return (0);
			}
		}

		// Double free
		if (HAS_POISON(ptr)) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (freed)\n", ptr);
			if (print_error())			aprintf(2, 0, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// In top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK) && heap->type != LARGE) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			if (print_error())			aprintf(2, 0, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// In middle chunk
		if (heap->type != LARGE) {
			t_chunk *next_chunk = GET_NEXT(chunk);
			if (!(next_chunk->size & PREV_INUSE)) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				if (print_error())		aprintf(2, 0, "Invalid pointer\n");
				abort_now();
				return (0);
			}
		}

		// Full chunk size
		size_t chunk_size = GET_SIZE(chunk);

		if (print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t[MALLOC_USABLE_SIZE] %d bytes available in chunk\n", ptr, chunk_size);

		return (chunk_size);
	}

#pragma endregion

#pragma region "Malloc Usable Size"

	__attribute__((visibility("default")))
	size_t malloc_usable_size(void *ptr) {
		ensure_init();

		if (!ptr) return (0);

		// malloc(0)
		if (check_digit(ptr, ZERO_MALLOC_BASE)) {
			return (0);
			mutex(&g_manager.mutex, MTX_LOCK);

				if ((uintptr_t)ptr % ALIGNMENT) {
					if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
					if (print_error())		aprintf(2, 0, "Invalid pointer\n");
					mutex(&g_manager.mutex, MTX_UNLOCK);
					abort_now(); return (0);
				}

				if (ptr < ZERO_MALLOC_BASE || ptr >= (void *)((char *)ZERO_MALLOC_BASE + (g_manager.alloc_zero_counter * ALIGNMENT))) {
					if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Invalid pointer (not allocated)\n", ptr);
					if (print_error())		aprintf(2, 0, "Invalid pointer\n");
					mutex(&g_manager.mutex, MTX_UNLOCK);
					abort_now(); return (0);
				}

				if (print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t   [FREE] Memory freed of size 0 bytes\n", ptr);
			
			mutex(&g_manager.mutex, MTX_UNLOCK);
			return (0);
		}

		t_arena	*arena = tcache;
		t_heap	*heap_ptr = NULL;
		size_t	chunk_size = 0;

		if (!arena) arena = &g_manager.arena;
		if (arena) {
			mutex(&arena->mutex, MTX_LOCK);

				if ((heap_ptr = heap_find(arena, ptr))) chunk_size = usable_ptr(ptr, heap_ptr);

			mutex(&arena->mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) {
			mutex(&g_manager.mutex, MTX_LOCK);

				t_arena *curr_arena = &g_manager.arena;
				while (curr_arena) {
					if (arena == curr_arena) { curr_arena = curr_arena->next; continue; }
					mutex(&curr_arena->mutex, MTX_LOCK);

						if ((heap_ptr = heap_find(curr_arena, ptr))) chunk_size = usable_ptr(ptr, heap_ptr);

					mutex(&curr_arena->mutex, MTX_UNLOCK);
					if (heap_ptr) break;
					curr_arena = curr_arena->next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) chunk_size = native_malloc_usable_size(ptr);

		return (chunk_size);
	}

#pragma endregion

#pragma region "Information"

	// Returns the usable size of a memory block.
	//
	//   size_t malloc_usable_size(void *ptr);
	//
	//   ptr – pointer returned by malloc/calloc/realloc.
	//
	// How it works:
	//   • Allocators often reserve more memory than requested, due to alignment or internal metadata.
	//   • When you request n bytes, the actual allocation may be larger.
	//   • malloc_usable_size(ptr) tells you how many usable bytes are available in that block.
	//
	//   • On success: returns the number of usable bytes available in the allocated block, which may be larger than the size originally requested.
	//   • On failure: if the pointer is NULL or invalid, the behavior is undefined.
	//
	// Notes:
	//   • The returned size is always >= the requested size (unless ptr is NULL).
	//   • Do NOT rely on the extra space — it's allocator-specific and may not be portable.
	//   • Passing an invalid or non-malloced pointer results in undefined behavior.

#pragma endregion
