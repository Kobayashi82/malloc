/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   internal.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 13:07:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 12:53:03 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "utils.h"

	#include <stdint.h>
	#include "unistd.h"
	#include <stdbool.h>
	#include <stdlib.h>
	#include <errno.h>
	#include <pthread.h>
	#include <dlfcn.h>
	#include <fcntl.h>
	#include <sys/mman.h>

	#ifdef _WIN32
		#include <windows.h>
	#endif

#pragma endregion

#pragma region "Defines"

	// ARCHITECTURE
	#define ARCHITECTURE				(sizeof(size_t) * 8)			// 32 or 64 bits
	
	#ifndef SIZE_MAX
		#define SIZE_MAX				~(size_t)0
	#endif
	#ifndef PATH_MAX
		#define PATH_MAX 				4096
	#endif
	
	// MAGIC & POISON
	#define MAGIC_BYTES					((size_t)0xABCDEF0123456789ULL)	// 4 or 8 bytes
	#define POISON_BYTES				((size_t)0xDEADBEEFCAFEBABEULL)	// 4 or 8 bytes
	#define HAS_MAGIC(ptr)				(*(const size_t *)((char *)(ptr) - sizeof(size_t)) == MAGIC_BYTES)
	#define HAS_POISON(ptr)				(*(const size_t *)((char *)(ptr) - sizeof(size_t)) == POISON_BYTES)
	#define SET_MAGIC(ptr)				(*(size_t *)((char *)(ptr) - sizeof(size_t)) = MAGIC_BYTES)
	#define SET_POISON(ptr)				(*(size_t *)((char *)(ptr) - sizeof(size_t)) = POISON_BYTES)

	// FD & BK
	#define GET_FD(chunk)				*(void **)((char *)(chunk) + sizeof(t_chunk))
	#define GET_BK(chunk)				*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(void *))
	#define SET_FD(chunk, next_chunk)	(*(void **)((char *)(chunk) + sizeof(t_chunk)) = (next_chunk))
	#define SET_BK(chunk, prev_chunk)	(*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(void *)) = (prev_chunk))

	// CHUNK
	#define GET_PTR(chunk)				(void *)((char *)(chunk) + sizeof(t_chunk))
	#define GET_HEAD(chunk) 			(void *)((char *)(chunk) - sizeof(t_chunk))
	#define GET_NEXT(chunk)				(t_chunk *)((char *)(chunk) + sizeof(t_chunk) + ((chunk)->size & ~15))
	#define GET_PREV(chunk)				(t_chunk *)((char *)(chunk) - (*(uint32_t *)((char *)(chunk) - sizeof(uint32_t)) + sizeof(t_chunk)))

	#define GET_PREV_SIZE(chunk)		(*(uint32_t *)((char *)(chunk) - sizeof(uint32_t)))
	#define SET_PREV_SIZE(chunk, size)	(*(uint32_t *)((char *)(chunk) - sizeof(uint32_t)) = (size))
	#define GET_SIZE(chunk) 			(size_t)((chunk)->size & ~15)

	// ALIGMENTS
	#define ZERO_MALLOC_BASE			(void *)0x100000000000					// 
	#define PAGE_SIZE					get_pagesize()					// 4096 bytes
	#define ALIGNMENT					(uint8_t)(ARCHITECTURE / 4)		// 8 or 16 bytes
	#define IS_ALIGNED(ptr)				(((uintptr_t)GET_HEAD(ptr) & (ALIGNMENT - 1)) == 0)
	#define ALIGN(size)					(((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

	// HEAPS
	#define MMAP_CHUNK    				0x8								// Bit 3
	#define TOP_CHUNK					0x4								// Bit 2
	#define HEAP_TYPE					0x2								// Bit 1
	#define PREV_INUSE					0x1								// Bit 0

	#define TINY_CHUNK					512
	#define TINY_BLOCKS					128
	#define TINY_SIZE					(((TINY_BLOCKS * TINY_CHUNK) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

	#define SMALL_CHUNK					4096
	#define SMALL_BLOCKS				128
	#define SMALL_SIZE					(((SMALL_BLOCKS * SMALL_CHUNK) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#pragma endregion

#pragma region "Enumerators"

	enum { TINY, SMALL, LARGE, FASTBIN, SMALLBIN, UNSORTEDBIN, LARGEBIN, MTX_INIT, MTX_LOCK, MTX_TRYLOCK, MTX_UNLOCK, MTX_DESTROY };

#pragma endregion

#pragma region "Structures"

	typedef struct s_chunk {
		size_t			size;						// Size of the chunk's user data
		size_t			magic;						// Magic or poison (depends on chunk status)
	} t_chunk;

	typedef struct s_heap_header {
		uint8_t 		total;						// 50 or 60 heaps (depends if there is arena data)
		uint8_t 		used;						// 
		void			*next;						// Pointer to next heap header
	} t_heap_header;

	typedef struct s_heap {
		void			*ptr;						// Pointer to the heap
		size_t			size;						// Size of the heap
		size_t			free;						// Memory available for allocation in the heap
		bool			active;						// Set to false when freed
		int				type;						// Type of the heap
		t_chunk			*top_chunk;					// Pointer to the top chunk
	} t_heap;

	typedef struct s_arena {
		int				id;
		int				alloc_count;				// Current number of allocations
		int				free_count;					// Current number of frees
		void			*fastbin[20];				// (8 - 160) or (16 - 160) bytes with 8 bytes increment (LIFO)
		void			*smallbin[31];				// (176-512 bytes para TINY, 513-4096 para SMALL) Doblemente enlazadas. Tamaños fijos (FIFO)
		void			*unsortedbin;				// ???
		void			*largebin[10];				// ???
		t_heap_header	*heap_header;				// 
		struct s_arena	*next;          			// Pointer to the next arena
		pthread_mutex_t	mutex;          			// Mutex for thread safety in the current arena
	} t_arena;

	typedef struct s_options {
		int				MXFAST;						// Max size (in bytes) for allocations using fastbins (0 disables fastbins)
		int				MIN_USAGE_PERCENT;			// Si una zona esta menos usada que esto, no usarla (pero si todas estan por debajo del threshold, usar la de mayor tamaño)
		int				CHECK_ACTION;				// Muestra informacion cuando hay errores
		unsigned char	PERTURB;					// Rellena la zona de memoria. malloc = ~PERTURB	free = PERTURB
		int				ARENA_TEST;					// 
		int				ARENA_MAX;					// Hard limit for arenas based on CPU count
		bool			DEBUG;						// 
		bool			LOGGING;					// 
		char 			LOGFILE[PATH_MAX];			// 
		int				fd_out;						// 
	} t_options;

	typedef struct s_manager {
		int				arena_count;				// Current number of arenas
		t_options		options;					// Options
		t_arena			arena;						// Main arena
		size_t			zero_malloc_counter;		// 
		pthread_mutex_t	mutex;						// Mutex for global synchronization
	} t_manager;

#pragma endregion

#pragma region "Variables"

	extern __thread t_arena	*tcache;				// 
	extern t_manager		g_manager;				// 

#pragma endregion

#pragma region "Methods"

	// Internal
	int		mutex(pthread_mutex_t *ptr_mutex, int action);
	void	*internal_alloc(size_t size);
	int		internal_free(void *ptr, size_t size);
	void	forksafe_init();
	void	ensure_init();
	size_t	get_pagesize();
	int		abort_now();

	// Options
	void	options_initialize();
	int		options_set(int param, int value);

	// Native
	void	native_free(void *ptr);
	void	*native_realloc(void *ptr, size_t size);
	size_t	native_malloc_usable_size(void *ptr);

#pragma endregion
