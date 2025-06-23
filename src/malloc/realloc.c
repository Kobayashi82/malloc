/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/24 01:05:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

	__attribute__((visibility("default")))
	void *calloc(size_t nmemb, size_t size) {
		void	*ptr = NULL;
		size_t	total = 0;

		if (!nmemb || !size)			return (malloc(0));
		if (nmemb > SIZE_MAX / size)	return (NULL);

		total = nmemb * size;
		if (g_manager.options.DEBUG) aprintf(1, "\t\t [CALLOC] Asking for %d bytes\n", size);
		if ((ptr = malloc(total))) ft_memset(ptr, 0, total);

		return (ptr);
	}

	void *realrealloc(void *ptr, size_t size) {
		#ifdef _WIN32
			static void* (__cdecl *real_realloc_win)(void*, size_t);
			if (!real_realloc_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_realloc_win = (void*(__cdecl*)(void*, size_t))GetProcAddress(m, "realloc");
			}

			if (!real_realloc_win) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate realloc to native allocator\n", ptr);
				return (NULL);
			}

			void *result = real_realloc_win(ptr, size);
			if (g_manager.options.DEBUG) aprintf(1, "%p->%p\t[REALLOC] Delegated to native allocator (size: %d)\n", ptr, result, size);
			return (result);
		#else
			static void* (*real_realloc_unix)(void*, size_t);
			if (!real_realloc_unix) real_realloc_unix = dlsym(((void *) -1L), "realloc");

			if (!real_realloc_unix) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate realloc to native allocator\n", ptr);
				return (NULL);
			}

			void *result = real_realloc_unix(ptr, size);
			if (g_manager.options.DEBUG) aprintf(1, "%p->%p\t[REALLOC] Delegated to native allocator (size: %d)\n", ptr, result, size);
			return (result);
		#endif
	}

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		if (g_manager.options.DEBUG) aprintf(1, "\t\t[REALLOC] Solicitando %d bytes\n", size);

		ensure_init();
		t_arena	*arena;
		void	*new_ptr = NULL;

		if (!ptr)	return malloc(size);				// ptr NULL equivale a malloc(size)
		if (!size)	return (free(ptr), NULL);			// size es 0 equivale a free(ptr)

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		// En la implementación real:
		// 1. Verificar si el chunk actual puede ser extendido
		// 2. Buscar en bins
		// 3. Determinar zona
		// 4. Crear asignacion en el top chunk
		// 5. Liberar antiguo chunk si es necesario (bins)

		t_heap	*heap_ptr = NULL;
		if (!(heap_ptr = heap_find(ptr, arena))) {
			return (realrealloc(ptr, size));
		}

		new_ptr = malloc(size);
		if (!new_ptr) return (NULL);


		// Trying something
		t_chunk *chunk = GET_HEAD(ptr);
		size_t copy_size = GET_SIZE(chunk); 							// must be ptr size
		if (size < copy_size) copy_size = size;
		ft_memcpy(new_ptr, ptr, copy_size);
		free(ptr);
		
		if (g_manager.options.DEBUG) aprintf(1, "%p\t[REALLOC] Memory reassigned from %p with %d bytes\n", new_ptr, ptr, size);

		return (new_ptr);
	}

#pragma endregion
