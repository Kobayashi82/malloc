/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/29 14:16:55 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"
	#include "utils.h"

#pragma endregion

#pragma region "Malloc"

	__attribute__((visibility("default")))
	void *malloc(size_t size) {
		// pthread_mutex_lock(&g_manager.mutex);
		// void *papa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
		// pthread_mutex_unlock(&g_manager.mutex);
		// return (papa);
		t_arena *arena;
		void *ptr = NULL;

		int debug_mode = g_manager.options.DEBUG;

		if (size == 0) return (NULL);
		if (debug_mode) ft_aprintf(1, "[MALLOC] Solicitando %d bytes\n", size);

		if (!thread_arena) {
			arena = arena_get();
			thread_arena = arena;
			if (!arena) {
				if (debug_mode) ft_aprintf(1, "[MALLOC] Error: No se pudo obtener arena\n");
				return (NULL);
			}
			if (debug_mode) ft_aprintf(1, "[MALLOC] Asignando arena #%d al hilo actual\n", arena->id);
		} else arena = thread_arena;
		
		pthread_mutex_lock(&arena->mutex);

			// En la implementación real:
			// 1. Determinar zona
			// 2. Determinar y posicion donde crear la asignacion 

			ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
			if (ptr == MAP_FAILED) { ptr = NULL;
				if (debug_mode) ft_aprintf(1, "[MALLOC] Error: No se pudo asignar memoria\n");
			} else if (debug_mode) {
				ft_aprintf(1, "[MALLOC] Asignados %d bytes en %p (arena #%d)\n", size, ptr, arena->id);
			} else {
				ft_aprintf(1, "Malloc Hilo: #%d\t\t\t(%p)\n", thread_arena->id, ptr);
			}

		pthread_mutex_unlock(&arena->mutex);

		return (ptr);
	}

#pragma endregion
