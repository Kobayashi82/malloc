/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/29 16:32:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"
	#include "utils.h"

#pragma endregion

#pragma region "Free"

	__attribute__((visibility("default")))
	void free(void *ptr) {
		if (!ptr) return;
		int debug_mode = g_manager.options.DEBUG;

		// Si el puntero es de nuestro range pero invalido, error y abort
		// Si no hay thread_arena o el puntero no es nuestro, return
		if (!thread_arena) {
			//ft_aprintf(2, "free(): invalid pointer\t\t(%p)\n", ptr);
			return;
		}

		if (debug_mode)
			ft_aprintf(1, "[FREE] Liberando memoria en %p\n", ptr);

		// Por ahora, solo se usara munmap
		// En la implementación real:
		// 1. Encontrar el bloque de memoria que contiene ptr
		// 2. Marcar ese bloque como libre
		// 3. Unir bloques libres adyacentes o zonas... no se aun

		if (munmap(ptr, 8) != 0)	; //ft_aprintf(1, "munmap failed Hilo: #%d\t\t(%p)\n", thread_arena->id, ptr);
		else if (debug_mode)		ft_aprintf(1, "[FREE] Memoria liberada en %p\n", ptr);
	}

#pragma endregion
