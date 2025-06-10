/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/10 14:30:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Real Free"

	void realfree(void *ptr) {
		#ifdef _WIN32
			static void (__cdecl *real_free_win)(void*);
			if (!real_free_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_free_win = (void(__cdecl*)(void*))GetProcAddress(m, "free");
			}
			if (!ptr) return;

			if (!real_free_win) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate free to native allocator\n", ptr);	
				return;
			}

			if (g_manager.options.DEBUG) aprintf(1, "%p\t   [FREE] Delegated to the native allocator\n", ptr);
			real_free_win(ptr);
		#else
			static void (*real_free_unix)(void*);
			if (!real_free_unix) real_free_unix = dlsym(((void *) -1L), "free");
			if (!ptr) return;

			if (!real_free_unix) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate free to native allocator\n", ptr);
				return;
			}

			if (g_manager.options.DEBUG) aprintf(1, "%p\t   [FREE] Delegated to the native allocator\n", ptr);
			real_free_unix(ptr);
		#endif
	}

#pragma endregion

#pragma region "Free PTR"

	static int free_ptr(void *ptr, t_arena *arena, t_heap *heap) {
		if (!ptr ||!arena || !heap) return (0);

		// LARGE
		if (heap->type == LARGE) {
			if ((void *)((char *)(ptr) - sizeof(t_lchunk)) == heap->ptr) {
				int result = heap_destroy(heap->ptr, heap->size, LARGE);
				if (result && g_manager.options.DEBUG)	aprintf(1, "%p\t   [ERROR] Unable to unmap memory\n", ptr);
				else if (g_manager.options.DEBUG)		aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);
				return (0);
			}

			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		// In top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK)) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		// Double free
		// 
		// magic number no sirve porque al coalescer pierdo el chunk
		// Una lista de los ultimos ptr no sirve porque se puede coalescer y crear otros
		// chunks que si se libera, esta en medio de otro chunk en uso o liberado
		// Quizas lo mas facil es invalid pointer siempre. Ningun mensaje de double free

		// In middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		heap->free += GET_SIZE(chunk) + sizeof(t_chunk);

		// Add to bins
		t_chunk_int chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		if (chunk_size <= (t_chunk_int)g_manager.options.MXFAST) {
			aprintf(1, "Fastbin\n");
			next_chunk->size &= ~PREV_INUSE;
			int index = (chunk_size - 1) / 8;
			GET_FD(chunk) = arena->fastbin[index];
			arena->fastbin[index] = (void *)chunk;
		} else {
			chunk = coalescing(chunk, arena, heap);
			if (!(chunk->size & TOP_CHUNK)) {
				aprintf(1, "Unsortedbin\n");
				// Add to unsortedbin
			}
		}

		arena->free_count++;
		if (heap->free == heap->size) {
			// Mark for elimination
		}

		if (g_manager.options.DEBUG)	aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);

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

				if ((heap_ptr = heap_find(ptr, arena))) free_ptr(ptr, arena, heap_ptr);
				
			mutex(&arena->mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) {
			mutex(&g_manager.mutex, MTX_LOCK);

				t_arena *curr_arena = &g_manager.arena;
				while (curr_arena) {
					if (arena == curr_arena) { curr_arena = curr_arena->next; continue; }
					mutex(&curr_arena->mutex, MTX_LOCK);

						if ((heap_ptr = heap_find(ptr, curr_arena))) free_ptr(ptr, curr_arena, heap_ptr);

					mutex(&curr_arena->mutex, MTX_UNLOCK);
					if (heap_ptr) break;
					curr_arena = curr_arena->next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) realfree(ptr);
	}

#pragma endregion
