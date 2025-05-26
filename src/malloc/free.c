/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/26 21:40:50 by vzurera-         ###   ########.fr       */
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

	void free_debug(int enable) { check_debug_env(enable); }

#pragma endregion

#pragma region "Realloc"

	void free(void *ptr) {
		int debug_mode = check_debug_env(2);

		if (!ptr) return;

		if (debug_mode)
			printf("[FREE] Liberando memoria en %p\n", ptr);

		// Por ahora, solo se usara munmap
		// En la implementación real:
		// 1. Encontrar el bloque de memoria que contiene ptr
		// 2. Marcar ese bloque como libre
		// 3. Unir bloques libres adyacentes o zonas... no se aun

		// Trying something
		munmap(ptr, 8);

		if (debug_mode) printf("[FREE] Memoria liberada en %p\n", ptr);
	}

#pragma endregion
