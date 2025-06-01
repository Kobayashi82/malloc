/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 18:48:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"
	#include "utils.h"

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
			if (!real_free_unix) real_free_unix = dlsym(RTLD_NEXT, "free");
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

		// Si el puntero no esta al inicio de un chunk
		//		Invalid pointer
		// Si el puntero ha sido liberado
		//		Double free
		// Si el puntero es parte del top chunk
		//		Free to no allocated
		// Si se libera correctamente (y DEBUG)
		//		if (g_manager.options.DEBUG)
		//			aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);

		// 1. Encontrar el bloque de memoria que contiene ptr
		// 2. Marcar ese bloque como libre (bins, etc)
		// Se usara munmap temporalmente

		if (heap->type == LARGE) {
			if (ptr == (void *)((char *)heap->ptr + sizeof(t_lchunk)))	heap_destroy(heap->ptr, heap->size, LARGE);
			else if (g_manager.options.DEBUG)							aprintf(1, "%p\t   [FREE] Invalid pointer\n", ptr);
		} else {
			if (munmap(ptr, 8)) {
				if (g_manager.options.DEBUG)
					aprintf(1, "%p\t  [ERROR] Unable to unmap memory\n", ptr);
				return (1);
			} else {
				if (g_manager.options.DEBUG)
					aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);
			}
			// free chunk
		}


		return (0);
	}
	
#pragma endregion

#pragma region "Free"

	__attribute__((visibility("default")))
	void free(void *ptr) {
		if (!ptr) return;

		t_arena	*arena = tcache;
		t_arena	*arena_ptr = NULL;
		t_heap	*heap_ptr = NULL;
		
		if (arena) {
			mutex(&arena->mutex, MTX_LOCK);

				heap_ptr = heap_find(ptr, arena);
				
			mutex(&arena->mutex, MTX_UNLOCK);
			if (heap_ptr) arena_ptr = arena;
		}

		if (!arena_ptr) {
			mutex(&g_manager.mutex, MTX_LOCK);

				t_arena *curr_arena = &g_manager.arena;
				while (curr_arena) {
					if (arena == curr_arena) { curr_arena = curr_arena->next; continue; }
					mutex(&curr_arena->mutex, MTX_LOCK);

						heap_ptr = heap_find(ptr, curr_arena);

					mutex(&curr_arena->mutex, MTX_UNLOCK);
					if (heap_ptr) { arena_ptr = curr_arena; break; }
					curr_arena = curr_arena->next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

		if (arena_ptr && heap_ptr) {
			free_ptr(ptr, arena_ptr, heap_ptr);
			return;
		}

		realfree(ptr);
	}

#pragma endregion
