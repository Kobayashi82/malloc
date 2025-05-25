/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 23:58:18 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/25 18:21:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// man mallopt para mas documentacion

//	Leer M_ARENA_TEST y M_ARENA_MAX de las variables (estas son para la funcion mallopt(), mirar man mallopt para las de entorno de verdad)

//	Si M_ARENA_TEST y M_ARENA_MAX no está establecido (0 o no existe):
//		M_ARENA_TEST = sizeof(long) == 4 ? 2 : 8;

//	Una vez que llegas a M_ARENA_TEST
//		M_ARENA_MAX = calculate_hardlimit();

//	Si M_ARENA_TEST no está presente pero M_ARENA_MAX sí está definido:
//		No calcular M_ARENA_TEST y usar el M_ARENA_MAX existente

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "Variables"

	t_arena_manager g_arena_manager;

#pragma endregion

#pragma region "Internal"

	#pragma region "Alloc"

		static void *internal_alloc(size_t size) {
			void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			return (ptr == MAP_FAILED ? NULL : ptr);
		}

	#pragma endregion

	#pragma region "Free"

		static void internal_free(void *ptr, size_t size) {
			if (ptr && size > 0) munmap(ptr, size);
		}

	#pragma endregion

	size_t get_pagesize() {
		#ifdef _WIN32
			SYSTEM_INFO info;
			GetSystemInfo(&info);
			return (info.dwPageSize);
		#else
			return ((size_t)sysconf(_SC_PAGESIZE));
		#endif
	}

	#pragma endregion

#pragma region "Arena"

	#pragma region "Initialize"

		#pragma region "Get CPUs"

			static int get_CPUs() {
				int CPUs = 1;

				#if defined(__linux__)
					CPUs = get_nprocs();
				#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
					size_t len = sizeof(CPUs);
					if (sysctlbyname("hw.ncpu", &CPUs, &len, NULL, 0)) CPUs = 1;
				#elif defined(_WIN32)
					SYSTEM_INFO sysinfo;
					GetSystemInfo(&sysinfo);
					CPUs = sysinfo.dwNumberOfProcessors;
				#elif defined(_SC_NPROCESSORS_ONLN)
					CPUs = sysconf(_SC_NPROCESSORS_ONLN);
				#endif

				return (CPUs <= 0 ? 1 : CPUs);
			}

		#pragma endregion

		#pragma region "Initialize"

			int arena_initialize() {
				g_arena_manager.cpu_count = get_CPUs();
				//g_arena_manager.arena_test = sizeof(long) == 4 ? 2 : 8;
				//g_arena_manager.arena_max = 0;
				g_arena_manager.arena_curr = 1;
				g_arena_manager.arenas = NULL;

				if (pthread_mutex_init(&g_arena_manager.mutex, NULL)) return (1);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Terminate"

		void arena_terminate() {
			t_arena *current, *next;

			pthread_mutex_lock(&g_arena_manager.mutex);

				current = g_arena_manager.arenas;
				while (current) {
					next = current->next;
					pthread_mutex_destroy(&current->mutex);

					// liberar zonas

					internal_free(current, sizeof(t_arena));
					current = next;
				}

			pthread_mutex_unlock(&g_arena_manager.mutex);
			pthread_mutex_destroy(&g_arena_manager.mutex);
		}

	#pragma endregion

	#pragma region "Get"

		t_arena *arena_get(size_t size) {
			t_arena *arena;

			if (!g_arena_manager.initialized) {
				if (!arena_initialize())		return (NULL);
				if (!g_arena_manager.arenas)	return ((arena = arena_create()) ? arena : NULL);
			}

			// Para asignaciones grandes, siempre usar la arena principal (la primera creada)
			if (size > SMALL_MAX) return (g_arena_manager.arenas);
			// Para asignaciones normales, intentamos reutilizar
			arena = arena_reuse();
			if (!arena) arena = arena_create();
			if (!arena) arena = g_arena_manager.arenas;

			return (arena);
		}

	#pragma endregion

	#pragma region "Create"

		#pragma region "Can Create"

			static int arena_can_create() {
				// if (g_arena_manager.arena_curr < g_arena_manager.arena_test) return (1);

				// if (!g_arena_manager.arena_max) {
				// 	g_arena_manager.arena_max = g_arena_manager.cpu_count * 2;
				// 	if (g_arena_manager.arena_max < 2) g_arena_manager.arena_max = g_arena_manager.arena_test;
				// }

				// return (g_arena_manager.arena_curr < g_arena_manager.arena_max);
				return (0);
			}

		#pragma endregion

		#pragma region "Create"

			t_arena *arena_create() {
				t_arena *new_arena;

				if (!arena_can_create()) return (NULL);

				new_arena = (t_arena*)internal_alloc(sizeof(t_arena));
				if (!new_arena) return (NULL);

				new_arena->id = g_arena_manager.arena_curr + 1;
				new_arena->active = 1;
				new_arena->used = 0;
				new_arena->next = NULL;
				new_arena->zones[0] = NULL;
				new_arena->zones[1] = NULL;
				new_arena->zones[2] = NULL;

				if (pthread_mutex_init(&new_arena->mutex, NULL)) {
					internal_free(new_arena, sizeof(t_arena));
					return (NULL);
				}
				
				pthread_mutex_lock(&g_arena_manager.mutex);

					new_arena->next = g_arena_manager.arenas;
					g_arena_manager.arenas = new_arena;
					g_arena_manager.arena_curr++;

				pthread_mutex_unlock(&g_arena_manager.mutex);

				return (new_arena);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Reuse"

		t_arena *arena_reuse() {
			t_arena *current, *best_arena = NULL;
			size_t min_usage = SIZE_MAX;

			if (!g_arena_manager.arenas) return (NULL);

			pthread_mutex_lock(&g_arena_manager.mutex);

				current = g_arena_manager.arenas;
				while (current) {
					if (current->active && current->used < min_usage) {
						min_usage = current->used;
						best_arena = current;
					}
					current = current->next;
				}

			pthread_mutex_unlock(&g_arena_manager.mutex);

			return (best_arena);
		}

	#pragma endregion

#pragma endregion