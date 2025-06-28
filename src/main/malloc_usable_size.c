/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc_usable_size.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 11:48:55 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 12:36:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Check Digit"

	static int check_digit(void *ptr1, void *ptr2) {
		if (!ptr1 || !ptr2) return (0);

		uintptr_t val1 = (uintptr_t)ptr1;
		uintptr_t val2 = (uintptr_t)ptr2;

		while (val1 >= 0x10) val1 /= 0x10;
		while (val2 >= 0x10) val2 /= 0x10;

		return ((val1 & 0xF) == (val2 & 0xF));
	}

#pragma endregion

#pragma region "Real Malloc Usable Size"

	static size_t real_malloc_usable_size(void *ptr) {
		if (!ptr) return 0;

		#ifdef _WIN32
			static size_t (__cdecl *real_malloc_usable_size_win)(void*);
			if (!real_malloc_usable_size_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_malloc_usable_size_win = (size_t(__cdecl*)(void*))GetProcAddress(m, "_msize");
			}

			if (!real_malloc_usable_size_win) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to native malloc_usable_size failed\n", ptr);
				return (0);
			}

			return (real_malloc_usable_size_win(ptr));
		#else
			static size_t (*real_malloc_usable_size_unix)(void*);
			if (!real_malloc_usable_size_unix) 
				real_malloc_usable_size_unix = dlsym(RTLD_NEXT, "malloc_usable_size");
			
			if (!real_malloc_usable_size_unix) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to native malloc_usable_size failed\n", ptr);
				return (0);
			}

			return (real_malloc_usable_size_unix(ptr));
		#endif
	}

#pragma endregion

#pragma region "Usable PTR"

	static size_t usable_ptr(void *ptr, t_heap *heap) {
		if (!ptr || !heap) return (0);

		// Not aligned
		if (!IS_ALIGNED(ptr)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// Heap freed
		if (!heap->active) {
			if (heap->type == LARGE && GET_PTR(heap->ptr) == ptr) {
				if		(g_manager.options.DEBUG)				aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (freed)\n", ptr);
				else if (g_manager.options.CHECK_ACTION != 2)	aprintf(2, "Invalid pointer\n");
			} else {
				if		(g_manager.options.DEBUG)				aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				else if (g_manager.options.CHECK_ACTION != 2)	aprintf(2, "Invalid pointer\n");
			}
			abort_now();
			return (0);
		}

		// LARGE
		if (heap->type == LARGE) {
				if (GET_HEAD(ptr) != heap->ptr) {
				if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
				abort_now();
				return (0);
			}
		}

		// Double free
		if (HAS_POISON(ptr)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (freed)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// In top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK) && heap->type != LARGE) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// In middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Invalid pointer\n");
			abort_now();
			return (0);
		}

		// Full chunk size
		size_t chunk_size = GET_SIZE(chunk);

		if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t[MALLOC_USABLE_SIZE] %d bytes available in chunk\n", ptr, chunk_size);

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
			mutex(&g_manager.mutex, MTX_LOCK);

				if ((uintptr_t)ptr % ALIGNMENT) {
					if		(g_manager.options.DEBUG)				aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
					else if (g_manager.options.CHECK_ACTION != 2)	aprintf(2, "Invalid pointer\n");
					mutex(&g_manager.mutex, MTX_UNLOCK);
					abort_now(); return (0);
				}

				if (ptr < ZERO_MALLOC_BASE || ptr >= (void *)((char *)ZERO_MALLOC_BASE + (g_manager.zero_malloc_counter * ALIGNMENT))) {
					if		(g_manager.options.DEBUG)				aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Invalid pointer (not allocated)\n", ptr);
					else if (g_manager.options.CHECK_ACTION != 2)	aprintf(2, "Invalid pointer\n");
					mutex(&g_manager.mutex, MTX_UNLOCK);
					abort_now(); return (0);
				}

				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t   [FREE] Memory freed of size 0 bytes\n", ptr);
			
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

		if (!heap_ptr && !check_digit(ptr, arena->heap_header)) chunk_size = real_malloc_usable_size(ptr);

		return (chunk_size);
	}

#pragma endregion

#pragma region "Information"

	// Returns the actual usable size of a memory block.
	// 
	// Memory allocators often allocate more memory than requested due to alignment.
	// This means that when you request n bytes, the allocator may reserve more space than you asked for.
	// 
	// malloc_usable_size() returns the total number of usable bytes available in that allocated block,
	// which may be equal to or greater than the size originally requested.

#pragma endregion
