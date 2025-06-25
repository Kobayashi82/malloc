/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 23:58:18 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/26 00:18:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Arena"

	#pragma region "Initialize"

		void arena_initialize(t_arena *arena) {
			arena->id = g_manager.arena_count + 1;
			arena->alloc_count = 0;
			arena->free_count = 0;
			ft_memset(arena->fastbin, 0, 20 * sizeof(void *));
			ft_memset(arena->smallbin, 0, 31 * sizeof(void *));
			ft_memset(arena->largebin, 0, 10 * sizeof(void *));
			arena->unsortedbin = NULL;
			arena->hiheap = NULL;
			arena->next = NULL;
			mutex(&arena->mutex, MTX_INIT);
			g_manager.arena_count++;
		}

	#pragma endregion

	#pragma region "Terminate"

		void arena_terminate2() {
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

			if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Library finalized\n");
		}

		#include <fcntl.h>

		void arena_terminate() {
			t_arena	*arena, *next;
			// t_heap	*heap;

			// if debug, print memoria status (free, not free, heaps, etc.)

			mutex(&g_manager.mutex, MTX_LOCK);

				arena = &g_manager.arena;
				while (arena) {
					mutex(&arena->mutex, MTX_LOCK);

						// Clean heaps

						// if (internal_free(curr, sizeof(t_heap))) {
						// 	result = 1;
						// 	if (g_manager.options.DEBUG)						aprintf(2, "\t\t  [ERROR] Failed to unmap heap structure\n");
						// }

						// heap = arena->tiny;
						// while (heap) {
						// 	if (heap_free(heap->ptr, heap->size, TINY, &arena->tiny)) break;
						// 	heap = arena->tiny;
						// }

						// heap = arena->small;
						// while (heap) {
						// 	if (heap_free(heap->ptr, heap->size, SMALL, &arena->small)) break;
						// 	heap = arena->small;
						// }

						// heap = arena->large;
						// while (heap) {
						// 	if (heap_free(heap->ptr, heap->size, LARGE, &arena->large)) break;
						// 	heap = arena->large;
						// }

						next = arena->next;
					
					mutex(&arena->mutex, MTX_UNLOCK);
					mutex(&arena->mutex, MTX_DESTROY);

					if (arena != &g_manager.arena) internal_free(arena, sizeof(t_arena));
					arena = next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
			mutex(&g_manager.mutex, MTX_DESTROY);

			// No funciona write en el destructor
			//
			// if (g_manager.options.DEBUG) {
			// 	int fd = open("/tmp/malloc_debug.log", O_CREAT | O_WRONLY | O_APPEND, 0644);
			// 	if (fd != -1) {
			// 		aprintf(fd, "\t\t [SYSTEM] Library finalized\n");
			// 		close(fd);
			// 	}
			// }

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
				if (!arena_can_create()) return (NULL);

				t_arena *new_arena = internal_alloc(PAGE_SIZE);
				arena_initialize((t_arena *)new_arena);

				t_arena *current = &g_manager.arena;
				while (current->next) current = current->next;
				current->next = new_arena;

				if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Arena #%d created\n", new_arena->id);

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
					if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Arena #%d locked\n", current->id);
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
					arena_initialize(&g_manager.arena);
					arena = &g_manager.arena;
					if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Arena #%d created\n", arena->id);
				}
				if (!arena) arena = arena_reuse();
				if (!arena) arena = arena_create();
				if (!arena) arena = &g_manager.arena;

				if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Arena #%d assigned\n", arena->id);

			mutex(&g_manager.mutex, MTX_UNLOCK);

			return (arena);
		}

	#pragma endregion

#pragma endregion
