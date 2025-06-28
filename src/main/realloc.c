/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 13:03:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Real Realloc"

	void *realrealloc(void *ptr, size_t size) {
		if (!ptr || !size) return (NULL);

		#ifdef _WIN32
			static void *(__cdecl *real_realloc_win)(void *, size_t);
			if (!real_realloc_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_realloc_win = (void*(__cdecl*)(void*, size_t))GetProcAddress(m, "realloc");
			}

			if (!real_realloc_win) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to the native realloc failed\n", ptr);
				return (NULL);
			}

			void *new_ptr = real_realloc_win(ptr, size);
		#else
			static void *(*real_realloc_unix)(void *, size_t);
			if (!real_realloc_unix) real_realloc_unix = dlsym(((void *) -1L), "realloc");

			if (!real_realloc_unix) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to the native realloc failed\n", ptr);
				return (NULL);
			}

			void *new_ptr = real_realloc_unix(ptr, size);
		#endif

		if (g_manager.options.DEBUG)			aprintf(g_manager.options.fd_out, "%p\t[REALLOC] Delegated to the native realloc from %p with %d bytes\n", new_ptr, ptr, size);
		return (new_ptr);
	}

#pragma endregion

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		ensure_init();

		t_arena	*arena;
		void	*new_ptr = NULL;

		if (!ptr)	return malloc(size);				// ptr NULL is equivalent to malloc(size)
		if (!size)	return (free(ptr), NULL);			// size 0 is equivalent to free(ptr)

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		mutex(&arena->mutex, MTX_LOCK);

			// tiene que buscar en todas las arenas
			if (!heap_find(arena, ptr)) {
				mutex(&arena->mutex, MTX_UNLOCK);
				free(ptr);
				return (realrealloc(ptr, size));
			}

			// Extend chunk or malloc(size)
			// Free old chunk if apply

			if (ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK)	new_ptr = heap_create(arena, LARGE, size);
			else												new_ptr = find_memory(arena, size);

			if (!new_ptr) {
				if ( g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to re-allocated %u bytes\n", size);
				mutex(&arena->mutex, MTX_UNLOCK);
				free(ptr);
				return (NULL);
			}

			t_chunk *chunk = GET_HEAD(ptr);
			size_t copy_size = GET_SIZE(chunk);
			if (size < copy_size) copy_size = size;
			ft_memcpy(new_ptr, ptr, copy_size);

			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t[REALLOC] Memory reassigned from %p with %u bytes\n", new_ptr, ptr, size);

			if (new_ptr) SET_MAGIC(new_ptr);

		mutex(&arena->mutex, MTX_UNLOCK);

		free(ptr);
		return (new_ptr);
	}

#pragma endregion
