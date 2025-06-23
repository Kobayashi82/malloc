/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:42:37 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/24 01:05:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "utils.h"
	#include "options.h"
	#include "heap.h"

	#include <pthread.h>
	#include <dlfcn.h>

	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <errno.h>
	#include <string.h>

	#ifdef _WIN32
		#include <windows.h>
	#endif

#pragma endregion

#pragma region "Variables"

	#pragma region "Defines"

		#define _GNU_SOURCE
		#define ARCHITECTURE				32 * ((sizeof(long) != 4) + 1)	// 32 or 64 bits
		#define PAGE_SIZE					get_pagesize()					// 4096 bytes
		#define ALIGNMENT					16								// 16 bytes
		#define POISON_BYTE					0xDE							// 1 byte
		#define CLEAN_BYTE					0x00							// 1 byte
		#define ARENAS_MAX					ARCHITECTURE * 2				// 64 or 128
		#define HEAPS_MAX					ARCHITECTURE * 4				// 128 or 256
		#define CHUNK_MIN					24								// 24
		#define INVALID_INDEX				~(unsigned char)0				// 255

	#pragma endregion

	#pragma region "Enumerators"

		typedef enum se_mutex { MTX_INIT, MTX_LOCK, MTX_TRYLOCK, MTX_UNLOCK, MTX_DESTROY } e_mutex;

	#pragma endregion

	#pragma region "Structures"

		typedef pthread_mutex_t	mtx_t;

		typedef struct s_arena {
			int				id;
			int				alloc_count;				// Current number of allocations
			int				free_count;					// Current number of frees
			void			*fastbin[20];				// (8 - 160) or (16 - 160) bytes with 8 bytes increment (LIFO)
			void			*smallbin[31];				// (176-512 bytes para TINY, 513-4096 para SMALL)		Doblemente enlazadas. Tamaños fijos (FIFO)
			void			*unsortedbin;				// ???
			void			*largebin[10];				// ???
			t_heap			*tiny;						// Linked list of TINY heaps
			t_heap			*small;						// Linked list of SMALL heaps
			t_heap			*large;						// Linked list of LARGE heaps (single chunk per heap)
			struct s_arena	*next;          			// Pointer to the next arena
			mtx_t			mutex;          			// Mutex for thread safety in the current arena
		} t_arena;

		typedef struct s_manager {
			int				arena_count;				// Current number of arenas
			t_options		options;					// Options
			t_arena			arena;						// Main arena
			mtx_t			mutex;						// Mutex for synchronizing access to the arenas
		} t_manager;

	#pragma endregion

	extern __thread t_arena	*tcache;
	extern t_manager		g_manager;

#pragma endregion

#pragma region "Methods"

	// Internal
	int		mutex(mtx_t *ptr_mutex, e_mutex action);
	void	*internal_alloc(size_t size);
	int		internal_free(void *ptr, size_t size);
	size_t	get_pagesize();
	void	ensure_init();

	// Arena
	int		arena_initialize(t_arena *arena);
	void	arena_terminate();
	t_arena *arena_get();

	// Coalescing
	int		link_chunk(t_chunk *chunk, t_chunk_int size, e_bintype type, t_arena *arena);
	int		unlink_chunk(t_chunk *chunk, t_arena *arena);
	t_chunk	*coalescing(t_chunk *chunk, t_arena *arena, t_heap *heap);

	// Bin
	void	*find_memory(t_arena *arena, size_t size);

	// Main
	void	realfree(void *ptr);
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);
	void	*calloc(size_t nmemb, size_t size);

	void	print_freelist_ranges();

#pragma endregion
