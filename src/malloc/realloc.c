/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/27 11:35:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		void *new_ptr;

		int debug_mode = g_arena_manager.options.DEBUG;

		// ptr NULL equivale a malloc(size)
		if (ptr == NULL) return malloc(size);

		// size es 0 equivale a free(ptr)
		if (size == 0) return (free(ptr), NULL);

		if (debug_mode) ft_printf(1, "[REALLOC] Reasignando memoria en %p a %d bytes\n", ptr, size);

		// En la implementación real:
		// 1. Verificar si el bloque actual puede ser extendido
		// 2. Si no, asignar un nuevo bloque y copiar los datos

		new_ptr = malloc(size);
		if (!new_ptr) {
			if (debug_mode) ft_printf(1, "[REALLOC] Error: No se pudo asignar memoria nueva\n");
			return (NULL);
		}

		// Trying something
		size_t copy_size = 64;
		if (size < copy_size) copy_size = size;
		memcpy(new_ptr, ptr, copy_size);
		free(ptr);

		if (debug_mode) ft_printf(1, "[REALLOC] Memoria reasignada de %p a %p (%d bytes)\n", ptr, new_ptr, size);

		return (new_ptr);
	}

#pragma endregion
