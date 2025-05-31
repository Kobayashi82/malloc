/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:07:54 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/31 14:11:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "options.h"
	#include "heap.h"
	#include "bin.h"

	#include <pthread.h>

	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/mman.h>
	#include <errno.h>
	#include <string.h>

#pragma endregion

#pragma region "Variables"

	#pragma region "Enumerators"

		typedef enum se_mutex { MTX_INIT, MTX_LOCK, MTX_TRYLOCK, MTX_UNLOCK, MTX_DESTROY } e_mutex;

	#pragma endregion

	#pragma region "Structures"

		typedef pthread_mutex_t	mtx_t;

		typedef struct s_arena t_arena;
		typedef struct s_arena {
			int			id;
			size_t		used;	    				// Memory used in the arena
			t_bin		fastbin[10];				// (16-160 bytes) incremento (sizeof(size_t) * 2))		Arrays de listas simples (LIFO)
			t_bin		unsortedbin[10];			// ???
			t_bin		smallbin[31];				// (176-512 bytes para TINY, 513-4096 para SMALL)		Doblemente enlazadas. Tamaños fijos (FIFO)
			t_bin		largebin[10];				// ???
			t_heap		*heap[3];    	  			// Memory zones (TINY, SMALL, LARGE)
			t_range		range;						// 
			mtx_t		mutex;          			// Mutex for thread safety in the current arena
			t_arena		*next;          			// Pointer to next arena
		} t_arena;

		typedef struct s_manager {
			bool		initialized;				// 
			bool		first_alloc;				// 
			int			arena_count;				// Current number of arenas created and active
			t_range		range[ARENAS_MAX + 1];		// 
			mtx_t		mutex;						// Mutex for synchronizing access to the arenas
			t_arena		arena;						// Main arena
			t_options	options;					// 
		} t_manager;

	#pragma endregion

	typedef struct s_tcache {
		t_arena		*arena;
	} t_tcache;

	extern __thread t_tcache	tcache;
	extern t_manager			g_manager;

#pragma endregion

#pragma region "Methods"

	int		mutex(mtx_t *ptr_mutex, e_mutex action);
	t_arena *arena_get();

#pragma endregion
