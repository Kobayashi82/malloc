/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:29:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/24 12:06:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stdio.h>
	#include <pthread.h>
	#include <stdlib.h>
	#include <sys/mman.h>
	#include <errno.h>
	#include <string.h>
	#include <unistd.h>

	#ifdef __linux__
		#include <sys/sysinfo.h>
	#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
		#include <sys/types.h>
		#include <sys/sysctl.h>
	#elif defined(_WIN32)
		#include <windows.h>
	#endif

	#ifndef SIZE_MAX
		#define SIZE_MAX ~(size_t)0
	#endif

	#define TINY_MAX		512
	#define SMALL_MAX		4096
	#define TINY_ZONE_SIZE	(get_pagesize() * 16)
	#define SMALL_ZONE_SIZE	(get_pagesize() * 128)

#pragma endregion

#pragma region "Variables"

	#pragma region "Enumerators"

		typedef enum se_zonetype { TINY, SMALL, LARGE } e_zonetype;
		typedef enum se_error {
			MTX_INIT = 20,
			MTX_LOCK = 21,
			MTX_UNLOCK = 22,
			MTX_DESTROY = 23,
			NO_MEMORY = 30
		} e_error;

	#pragma endregion

	#pragma region "Structures"

		typedef pthread_mutex_t	mtx_t;
		typedef struct s_zone	t_zone;
		typedef struct s_arena	t_arena;

		typedef struct s_zone {
			e_zonetype	type;			// Type
			// header?
			size_t		size;			// Zone size
			size_t		blocks;			// Number of blocks
			size_t		block_size;		// Block size
			t_zone		*next;			// pointer to next zone
		} t_zone;

		typedef struct s_arena {
			int			id;             // ID
			int			active;         // Active
			// header?
			size_t		used;    		// Memory used
			t_zone		*zones[3];      // Zones (TINY, SMALL, LARGE)
			mtx_t		mutex;          // Arena mutex
			t_arena		*next;          // Pointer to next arena
		} t_arena;

		typedef struct s_arena_manager {
			int			cpu_count;		// CPUs
			int			arena_test;		// M_ARENA_TEST
			int			arena_max;		// M_ARENA_MAX
			int			arena_curr;		// Number of arenas created
			mtx_t		mutex;			// Global mutex
			t_arena		*arenas;		// Pointer to arenas
		} t_arena_manager;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	extern t_arena_manager	*g_arena_manager;

	size_t	get_pagesize();

	// Arena
	int		arena_initialize();
	void	arena_terminate();
	t_arena *arena_get(size_t size);
	t_arena *arena_create();
	t_arena *arena_reuse();

	// Main functions
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);

#pragma endregion
