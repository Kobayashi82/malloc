/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/03 19:11:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Malloc"

	__attribute__((visibility("default")))
	void *malloc(size_t size) {
		t_arena	*arena;
		void	*ptr = NULL;

		if (!size) size = 1;
		// if (g_manager.options.DEBUG) aprintf(1, "\t\t [MALLOC] Solicitando %d bytes\n", size);

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

			// En la implementación real:
			// 1. Buscar en bins
			// 2. Determinar zona
			// 3. Crear asignacion en el top chunk

			// Test malloc fail
			// ptr = &arena;
			// return (ptr);
			if (size > SMALL_SIZE) {
				ptr = heap_create(LARGE, size);
			} else {
				ptr = heap_create(LARGE, size);
				if		(!ptr && g_manager.options.DEBUG)	aprintf(1, "\t\t  [ERROR] Failed to allocated %d bytes\n", size);
				else if (g_manager.options.DEBUG)			aprintf(1, "%p\t [MALLOC] Allocated %d bytes\n", ptr, size);
			}

		return (ptr);
	}

#pragma endregion
