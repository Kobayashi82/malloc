/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/04 11:24:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		t_arena	*arena;
		void	*new_ptr = NULL;

		if (g_manager.options.DEBUG) aprintf(1, "\t\t[REALLOC] Solicitando %d bytes\n", size);

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

		new_ptr = malloc(size);
		if (!new_ptr) return (NULL);

		// Trying something
		size_t copy_size = 64; 							// must be ptr size
		if (size < copy_size) copy_size = size;
		ft_memcpy(new_ptr, ptr, copy_size);
		free(ptr);
		
		if (g_manager.options.DEBUG) aprintf(1, "%p\t[REALLOC] Memory reassigned from %p with %d bytes\n", new_ptr, ptr, size);

		return (new_ptr);
	}

#pragma endregion
