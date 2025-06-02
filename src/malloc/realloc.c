/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/02 14:01:25 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		void *new_ptr;

		// ptr NULL equivale a malloc(size)
		if (ptr == NULL) return malloc(size);

		// size es 0 equivale a free(ptr)
		if (size == 0) return (free(ptr), NULL);

		// if (g_manager.options.DEBUG) aprintf(1, "%p\t[REALLOC] Reasignando memoria en %p a %d bytes\n", ptr, size);

		// En la implementación real:
		// 1. Verificar si el bloque actual puede ser extendido
		// 2. Si no, asignar un nuevo bloque y copiar los datos

		new_ptr = malloc(size);
		if (!new_ptr) {
			if (g_manager.options.DEBUG) aprintf(1, "\t\t[REALLOC] Error: No se pudo asignar memoria nueva\n");
			return (NULL);
		}

		// Trying something
		size_t copy_size = 64;
		if (size < copy_size) copy_size = size;
		memcpy(new_ptr, ptr, copy_size);
		free(ptr);

		if (g_manager.options.DEBUG) aprintf(1, "%p\t[REALLOC] Memoria reasignada a %p (%d bytes)\n", ptr, new_ptr, size);

		return (new_ptr);
	}

#pragma endregion
