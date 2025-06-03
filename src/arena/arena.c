/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 23:58:18 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/03 21:54:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma endregion

#pragma region "Arena"

	#pragma region "Initialize"

		int arena_initialize(t_arena *arena) {
			arena->id = g_manager.arena_count + 1;
			ft_memset(arena->fastbin, 0, sizeof(t_chunk_int) * 20);
			// bins
			arena->tiny = NULL;
			arena->small = NULL;
			arena->large = NULL;
			arena->next = NULL;
			mutex(&arena->mutex, MTX_INIT);
			g_manager.arena_count++;

			if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Library initialized\n");
			return (0);
		}

	#pragma endregion

	#pragma region "Terminate"

		void arena_terminate() {
			t_arena *current, *next;

			mutex(&g_manager.mutex, MTX_LOCK);

				mutex(&g_manager.arena.mutex, MTX_DESTROY);
				current = g_manager.arena.next;
				while (current) {
					mutex(&current->mutex, MTX_DESTROY);

					// liberar zonas

					next = current->next;
					internal_free(current, sizeof(t_arena));
					current = next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
			mutex(&g_manager.mutex, MTX_DESTROY);

			if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Library finalized\n");
		}

	#pragma endregion

	#pragma region "Create"

		#pragma region "Get CPUs"

			int get_CPUs() {
				int CPUs = 1;

				#ifdef _WIN32
					SYSTEM_INFO sysinfo;
					GetSystemInfo(&sysinfo);
					CPUs = sysinfo.dwNumberOfProcessors;
				#else
					CPUs = sysconf(_SC_NPROCESSORS_ONLN);
				#endif

				return (CPUs <= 0 ? 1 : CPUs);
			}

		#pragma endregion

		#pragma region "Can Create"

			static int arena_can_create() {
				if (!g_manager.options.ARENA_MAX) {
					if (g_manager.arena_count >= g_manager.options.ARENA_TEST) {
						g_manager.options.ARENA_MAX = ft_min(get_CPUs() * 2, ARENAS_MAX);
						return (g_manager.arena_count < g_manager.options.ARENA_MAX);
					} else return (1);
				}
				return (g_manager.arena_count < g_manager.options.ARENA_MAX);
			}

		#pragma endregion

		#pragma region "Create"

			t_arena *arena_create() {
				t_arena *new_arena;

				if (!arena_can_create()) return (NULL);

				new_arena = internal_alloc(sizeof(t_arena));
				if (!new_arena) return (NULL);

				if (arena_initialize(new_arena)) {
					internal_free(new_arena, sizeof(t_arena));
					return (NULL);
				}

				t_arena *current = &g_manager.arena;
				while (current->next) current = current->next;
				current->next = new_arena;

				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d created\n", new_arena->id);

				return (new_arena);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Reuse"

		t_arena *arena_reuse() {
			t_arena *current, *best_arena = NULL;

			current = &g_manager.arena;
			while (current) {
				if (!mutex(&current->mutex, MTX_TRYLOCK)) {
						best_arena = current;
					mutex(&current->mutex, MTX_UNLOCK);
					// return (NULL);	// Force arena creation
					break;
				} else {
					if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d locked\n", current->id);
				}
				current = current->next;
			}

			return (best_arena);
		}

	#pragma endregion

	#pragma region "Get"

		t_arena *arena_get() {
			static bool initialized;
			t_arena *arena = NULL;

			mutex(&g_manager.mutex, MTX_LOCK);

				if (!initialized) {
					initialized = true;
					if (arena_initialize(&g_manager.arena)) abort();
					arena = &g_manager.arena;
					if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d created\n", arena->id);
				}
				if (!arena) arena = arena_reuse();
				if (!arena) arena = arena_create();
				if (!arena) arena = &g_manager.arena;

				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d assigned\n", arena->id);

			mutex(&g_manager.mutex, MTX_UNLOCK);

			return (arena);
		}

	#pragma endregion

#pragma endregion
