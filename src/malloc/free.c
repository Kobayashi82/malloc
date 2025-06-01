/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 15:05:24 by vzurera-         ###   ########.fr       */
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
				return ;
			}

			if (g_manager.options.DEBUG) aprintf(1, "%p\t   [FREE] Delegated to the native allocator\n", ptr);
			real_free_win(ptr);
		#else
			static void (*real_free_unix)(void*);
			if (!real_free_unix) real_free_unix = dlsym(RTLD_NEXT, "free");
			if (!ptr) return;

			if (!real_free_unix) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate free to native allocator\n", ptr);
				return ;
			}

			if (g_manager.options.DEBUG) aprintf(1, "%p\t   [FREE] Delegated to the native allocator\n", ptr);
			real_free_unix(ptr);
		#endif
	}

#pragma endregion

#pragma region "Free PTR"

	int free_ptr(t_arena *arena, void *ptr) {
		if (!arena || !ptr) return (0);

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

		if (munmap(ptr, 8)) {
			if (g_manager.options.DEBUG)
				aprintf(1, "%p\t  [ERROR] Unable to unmap memory\n", ptr);
			return (1);
		} else {
			if (g_manager.options.DEBUG)
				aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);
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
		
		if (arena) {
			mutex(&arena->mutex, MTX_LOCK);

				// BUSCAR EN tiny, small y large

				// for (int i = 0; i < HEAPS_MAX; ++i) {
				// 	if (!arena->range.start[i] || !arena->range.end[i]) break;
				// 	if (ptr >= arena->range.start[i] && ptr <= arena->range.end[i]) {
				// 		free_ptr(arena, ptr);
				// 		mutex(&arena->mutex, MTX_UNLOCK);
				// 		return;
				// 	}
				// }

			mutex(&arena->mutex, MTX_UNLOCK);
		}

		if (!arena_ptr) {
			mutex(&g_manager.mutex, MTX_LOCK);

				// BUSCAR EN tiny, small y large de cada arena menos la actual

				// for (int a = 0; a < ARENAS_MAX; ++a) {
				// 	if (arena && arena->id - 1 == a) continue;
				// 	for (int i = 0; i < HEAPS_MAX; ++i) {
				// 		if (!g_manager.range[a].start[i] || !g_manager.range[a].end[i]) break;
				// 		if (ptr >= g_manager.range[a].start[i] && ptr <= g_manager.range[a].end[i]) {
				// 			arena_ptr = &g_manager.arena;
				// 			while (arena_ptr && arena_ptr->id - 1 != a)
				// 				arena_ptr = arena_ptr->next;
				// 			if (arena_ptr) {
				// 				mutex(&arena_ptr->mutex, MTX_LOCK);
				// 				mutex(&g_manager.mutex, MTX_UNLOCK);

				// 					free_ptr(arena_ptr, ptr);

				// 				mutex(&arena_ptr->mutex, MTX_UNLOCK);
				// 				return;
				// 			}
				// 		}
				// 	}
				// }

			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

		return;			// Prevent segfault in real free (for now)
		realfree(ptr);
	}

#pragma endregion
