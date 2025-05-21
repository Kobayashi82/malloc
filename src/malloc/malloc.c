/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/21 16:11:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "Variables"

	static __thread t_arena *thread_arena;

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

	void malloc_debug(int enable) { check_debug_env(enable); }

#pragma endregion

#pragma region "Constructor"

	__attribute__((constructor)) static void malloc_initialize() {
		check_debug_env(2);
		arena_initialize();
		if (check_debug_env(2)) printf("[MALLOC_INIT] Sistema de arenas inicializado\n");
	}

	__attribute__((destructor)) static void malloc_terminate() { arena_terminate(); }


#pragma endregion

#pragma region "Malloc"

	void *malloc(size_t size) {
		t_arena *arena;
		void *ptr = NULL;

		int debug_mode = check_debug_env(2);

		if (size == 0) return (NULL);

		if (debug_mode) printf("[MALLOC] Solicitando %zu bytes\n", size);

		if (!thread_arena) {
			arena = arena_get(size);
			if (!arena) {
				if (debug_mode) printf("[MALLOC] Error: No se pudo obtener arena\n");
				return (NULL);
			}
			thread_arena = arena;
			if (debug_mode) printf("[MALLOC] Asignando arena #%d al hilo actual\n", arena->id);
		} else arena = thread_arena;

		pthread_mutex_lock(&arena->mutex);

			// En la implementación real:
			// 1. Determinar zona
			// 2. Determinar y posicion donde crear la asignacion 

			ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
			if (ptr == MAP_FAILED) { ptr = NULL;
				if (debug_mode) printf("[MALLOC] Error: No se pudo asignar memoria\n");
			} else if (debug_mode) {
				printf("[MALLOC] Asignados %zu bytes en %p (arena #%d)\n", size, ptr, arena->id);
			}

		pthread_mutex_unlock(&arena->mutex);

		return (ptr);
	}

#pragma endregion
