/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:07:54 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 15:03:18 by vzurera-         ###   ########.fr       */
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
			t_bin		fastbin[10];				// (16-160 bytes) incremento (sizeof(size_t) * 2))		Arrays de listas simples (LIFO)
			t_bin		unsortedbin[10];			// ???
			t_bin		smallbin[31];				// (176-512 bytes para TINY, 513-4096 para SMALL)		Doblemente enlazadas. Tamaños fijos (FIFO)
			t_bin		largebin[10];				// ???
			t_heap		*tiny;						// Linked list of TINY heaps
			t_heap		*small;						// Linked list of SMALL heaps
			t_heap		*large;						// Linked list of LARGE heaps (single chunk per heap)
			t_arena		*next;          			// Pointer to the next arena
			mtx_t		mutex;          			// Mutex for thread safety in the current arena
		} t_arena;

		typedef struct s_manager {
			bool		initialized;				// 
			bool		first_alloc;				// 
			int			arena_count;				// Current number of arenas created and active
			t_options	options;					// 
			t_arena		arena;						// Main arena
			mtx_t		mutex;						// Mutex for synchronizing access to the arenas
		} t_manager;

	#pragma endregion

	extern __thread t_arena	*tcache;
	extern t_manager		g_manager;

#pragma endregion

#pragma region "Methods"

	int		mutex(mtx_t *ptr_mutex, e_mutex action);
	t_arena *arena_get();

#pragma endregion
