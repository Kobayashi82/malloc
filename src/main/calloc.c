/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 12:25:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 13:02:51 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Calloc"

	__attribute__((visibility("default")))
	void *calloc(size_t nmemb, size_t size) {
		ensure_init();

		t_arena	*arena;
		void	*ptr = NULL;
		size_t	total = 0;

		if (!nmemb || !size)			return (malloc(0));
		if (nmemb > SIZE_MAX / size)	return (NULL);

		total = nmemb * size;

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		mutex(&arena->mutex, MTX_LOCK);

			if (ALIGN(total + sizeof(t_chunk)) > SMALL_CHUNK) ptr = heap_create(arena, LARGE, total);
			else {
				ptr = find_memory(arena, total);
				if (ptr) ft_memset(ptr, 0, total);
			}

			if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [CALLOC] Allocated %u bytes\n", ptr, size);
			else if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

			if (ptr) SET_MAGIC(ptr);

		mutex(&arena->mutex, MTX_UNLOCK);

		return (ptr);
	}

#pragma endregion
