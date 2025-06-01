/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 18:49:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"
	#include "utils.h"

#pragma endregion

#pragma region "Malloc"

	__attribute__((visibility("default")))
	void *malloc(size_t size) {
		t_arena *arena;
		void *ptr = NULL;

		if (size == 0) return (NULL);
		// if (g_manager.options.DEBUG) aprintf(1, "\t\t [MALLOC] Solicitando %d bytes\n", size);

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t [MALLOC] Error: No se pudo obtener arena\n");
				return (NULL);
			}
		} else arena = tcache;

			// En la implementación real:
			// 1. Determinar zona
			// 2. Determinar y posicion donde crear la asignacion

			if (size > SMALL_SIZE) {
				ptr = heap_create(LARGE, size);
			} else {
				ptr = heap_create(LARGE, size);
			}

		if (g_manager.options.DEBUG) aprintf(1, "%p\t [MALLOC] Allocated %d bytes\n", ptr, size);

		return (ptr);
	}

#pragma endregion
