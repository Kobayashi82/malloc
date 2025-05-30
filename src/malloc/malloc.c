/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/30 19:04:19 by vzurera-         ###   ########.fr       */
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

		if (!tcache.arena) {
			arena = arena_get();
			tcache.arena = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t [MALLOC] Error: No se pudo obtener arena\n");
				return (NULL);
			}
		} else arena = tcache.arena;
		
		mutex(&arena->mutex, MTX_LOCK);

			// En la implementación real:
			// 1. Determinar zona
			// 2. Determinar y posicion donde crear la asignacion 

			ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
			if (ptr == MAP_FAILED) { ptr = NULL;
				if (g_manager.options.DEBUG) aprintf(1, "\t\t [MALLOC] Error: No se pudo asignar memoria\n");
			} else if (g_manager.options.DEBUG) {
				aprintf(1, "%p\t [MALLOC] Allocated %d bytes\n", ptr, size);
			}

		mutex(&arena->mutex, MTX_UNLOCK);

		return (ptr);
	}

#pragma endregion
