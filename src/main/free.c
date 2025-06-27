/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/27 17:09:09 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Real Free"

	void realfree(void *ptr) {
		if (!ptr) return;

		#ifdef _WIN32
			static void (__cdecl *real_free_win)(void*);
			if (!real_free_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_free_win = (void(__cdecl*)(void*))GetProcAddress(m, "free");
			}

			if (!real_free_win) {
				if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Delegation to the native free failed\n", ptr);
				return;
			}

			real_free_win(ptr);
		#else
			static void (*real_free_unix)(void*);
			if (!real_free_unix) real_free_unix = dlsym(((void *) -1L), "free");

			if (!real_free_unix) {
				if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Delegation to the native free failed\n", ptr);
				return;
			}

			real_free_unix(ptr);
		#endif

		if (g_manager.options.DEBUG)			aprintf(2, "%p\t   [FREE] Delegated to the native free\n", ptr);
	}

#pragma endregion

#pragma region "Free PTR"

	static int free_ptr(t_arena *arena, void *ptr, t_heap *heap) {
		if (!ptr ||!arena || !heap) return (0);

		// Not aligned
		if (!IS_ALIGNED(ptr)) {
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
			else							aprintf(2, "Invalid pointer\n");
			abort();
		}

		// Heap freed
		if (!heap->active) {
			if (heap->type == LARGE) {
				if ((void *)((char *)heap->ptr + sizeof(t_chunk)) == ptr) {
					if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Double free\n", ptr);
					else							aprintf(2, "Double free\n");
				} else {
					if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
					else							aprintf(2, "Invalid pointer\n");			
				}
			} else {
				if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				else							aprintf(2, "Invalid pointer\n");
			}
			abort();
		}

		// LARGE
		if (heap->type == LARGE) {
			if ((void *)((char *)(ptr) - sizeof(t_chunk)) == heap->ptr) {
				int result = heap_destroy(arena, heap->ptr, LARGE, heap->size);
				if (result && g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Unable to unmap memory\n", ptr);
				else if (g_manager.options.DEBUG)		aprintf(2, "%p\t   [FREE] Memory freed\n", ptr);
				return (0);
			}

			if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else							aprintf(2, "Invalid pointer\n");
			abort();
		}

		// Double free
		if (HAS_POISON(ptr)) {
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Double free\n", ptr);
			else							aprintf(2, "Double free\n");
			abort();
		}

		// In top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK)) {
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			else							aprintf(2, "Invalid pointer\n");
			abort();
		}

		// In middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) {
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else							aprintf(2, "Invalid pointer\n");
			abort();
		}

		SET_POISON(ptr);
		heap->free += GET_SIZE(chunk) + sizeof(t_chunk);

		// Add to bins
		next_chunk->size &= ~PREV_INUSE;
		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		SET_PREV_SIZE(next_chunk, GET_SIZE(chunk));
		if (chunk_size <= (size_t)g_manager.options.MXFAST) {
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t [SYSTEM] Chunk added to FastBin\n", chunk);
			link_chunk(chunk, chunk_size, FASTBIN, arena);
		} else {
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t [SYSTEM] Coalescing adjacent chunks\n", chunk);
			chunk = coalescing(chunk, arena, heap);
			if (g_manager.options.DEBUG)	aprintf(2, "%p\t [SYSTEM] Chunk added to UnsortedBin\n", chunk);
			link_chunk(chunk, chunk_size, UNSORTEDBIN, arena);
		}

		arena->free_count++;
		if (heap->free == heap->size) {
			// Mark for elimination
		}

		if (g_manager.options.DEBUG)	aprintf(2, "%p\t   [FREE] Memory freed of size %d bytes\n", ptr, chunk_size);

		return (0);
	}
	
#pragma endregion

#pragma region "Free"

	__attribute__((visibility("default")))
	void free(void *ptr) {
		ensure_init();

		if (!ptr) return;

		t_arena	*arena = tcache;
		t_heap	*heap_ptr = NULL;
		
		if (!arena) arena = &g_manager.arena;
		if (arena) {
			mutex(&arena->mutex, MTX_LOCK);

				if ((heap_ptr = heap_find(arena, ptr))) free_ptr(arena, ptr, heap_ptr);
				
			mutex(&arena->mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) {
			mutex(&g_manager.mutex, MTX_LOCK);

				t_arena *curr_arena = &g_manager.arena;
				while (curr_arena) {
					if (arena == curr_arena) { curr_arena = curr_arena->next; continue; }
					mutex(&curr_arena->mutex, MTX_LOCK);

						if ((heap_ptr = heap_find(curr_arena, ptr))) free_ptr(curr_arena, ptr, heap_ptr);

					mutex(&curr_arena->mutex, MTX_UNLOCK);
					if (heap_ptr) break;
					curr_arena = curr_arena->next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) {
			if (arena->heap_header) {
				uintptr_t val1 = (uintptr_t)ptr;
				uintptr_t val2 = (uintptr_t)arena->heap_header;

				while (val1 >= 0x10) val1 /= 0x10;
				while (val2 >= 0x10) val2 /= 0x10;

				if (!((val1 & 0xF) == (val2 & 0xF)))	realfree(ptr);
			} else if (g_manager.options.DEBUG)			aprintf(2, "%p\t  [ERROR] Invalid pointer (not allocated)\n", ptr);
			else										aprintf(2, "Invalid pointer\n");
		}
	}

#pragma endregion
