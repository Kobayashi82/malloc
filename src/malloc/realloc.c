/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/26 21:40:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "Debug"

	static int check_debug_env(int enable) {
		static int debug_mode = -1;

		if (debug_mode == -1) {
			enable = 0;
			const char *debug_env = getenv("MALLOC_DEBUG");
			if (debug_env && (!strcmp(debug_env, "1") || !strcmp(debug_env, "yes") || !strcmp(debug_env, "true"))) {
				enable = 1;
				printf("[MALLOC] Debug activado mediante variable de entorno\n");
			}
		}

		if (enable == 0 || enable == 1) debug_mode = enable;
		return (debug_mode);
	}

	void realloc_debug(int enable) { check_debug_env(enable); }

#pragma endregion

#pragma region "Realloc"

	void *realloc(void *ptr, size_t size) {
		void *new_ptr;

		int debug_mode = check_debug_env(2);

		// ptr NULL equivale a malloc(size)
		if (ptr == NULL) return malloc(size);

		// size es 0 equivale a free(ptr)
		if (size == 0) return (free(ptr), NULL);

		if (debug_mode) printf("[REALLOC] Reasignando memoria en %p a %zu bytes\n", ptr, size);

		// En la implementación real:
		// 1. Verificar si el bloque actual puede ser extendido
		// 2. Si no, asignar un nuevo bloque y copiar los datos

		new_ptr = malloc(size);
		if (!new_ptr) {
			if (debug_mode) printf("[REALLOC] Error: No se pudo asignar memoria nueva\n");
			return (NULL);
		}

		// Trying something
		size_t copy_size = 64;
		if (size < copy_size) copy_size = size;
		memcpy(new_ptr, ptr, copy_size);
		free(ptr);

		if (debug_mode) printf("[REALLOC] Memoria reasignada de %p a %p (%zu bytes)\n", ptr, new_ptr, size);

		return (new_ptr);
	}

#pragma endregion
