/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:07:54 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/28 18:50:55 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "options.h"
	#include "heap.h"
	#include "bin.h"

	#include <pthread.h>

#pragma endregion

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
		mtx_t		mutex;          			// Mutex for thread safety in the current arena
		t_arena		*next;          			// Pointer to next arena
	} t_arena;

	typedef struct s_manager {
		bool		initialized;				// 
		bool		first_alloc;				// 
		int			arena_count;				// Current number of arenas created and active
		mtx_t		mutex;						// Mutex for synchronizing access to the arenas
		t_arena		arena;						// Main arena
		t_options	options;					// 
	} t_manager;

	extern __thread t_arena	*thread_arena;
	extern t_manager		g_manager;

	t_arena *arena_get();


	// ARENA
	// Estructura compartida entre hilos que contiene HEAPs y BINs

	// HEAP
	// Region de memoria contigua que se divide en CHUNKs

	// CHUNK
	// Porción de memoria que envuelve un BLOCK y contiene METADATA
	
	// METADATA
	// Informacion sobre una BLOCK y que se encuentra antes y despues de un BLOCK

	// BLOCK
	// Region dentro de un CHUNK y perteneciente a un HEAP donde se guardan datos. Es lo que devuelve malloc y libera free

	// BIN
	// Listas enlazadas con informacion a CHUNKs libres. Se usa para la reutilizacion de espacios de memoria

	// TCACHE
	// Array de listas enlazadas con informacion a CHUNKs libres. Se usa para para la reutilizacion de espacios de memoria, pero cada hilo tiene su propio TCACHE
	