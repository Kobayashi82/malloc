/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/27 11:35:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "Free"

	__attribute__((visibility("default")))
	void free(void *ptr) {
		int debug_mode = g_arena_manager.options.DEBUG;

		if (!ptr) return;

		if (debug_mode)
			ft_printf(1, "[FREE] Liberando memoria en %p\n", ptr);

		// Por ahora, solo se usara munmap
		// En la implementación real:
		// 1. Encontrar el bloque de memoria que contiene ptr
		// 2. Marcar ese bloque como libre
		// 3. Unir bloques libres adyacentes o zonas... no se aun

		// Trying something
		munmap(ptr, 8);

		if (debug_mode) ft_printf(1, "[FREE] Memoria liberada en %p\n", ptr);
	}

#pragma endregion
