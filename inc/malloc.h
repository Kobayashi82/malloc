/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:29:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/25 19:23:14 by vzurera-         ###   ########.fr       */
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
	#include <stdint.h>

#pragma endregion

#pragma region "Defines"

	#ifdef __linux__
		#include <sys/sysinfo.h>
	#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
		#include <sys/types.h>
		#include <sys/sysctl.h>
	#elif defined(_WIN32)
		#include <windows.h>
	#endif

	#define M_MXFAST			1	//
	#define M_TRIM_THRESHOLD	-1	//
	#define M_TOP_PAD			-2	//
	#define M_MMAP_THRESHOLD	-3	//
	#define M_MMAP_MAX			-4	//
	#define M_CHECK_ACTION		-5	//
	#define M_PERTURB			-6	//
	#define M_ARENA_TEST		-7	//
	#define M_ARENA_MAX			-8	//

	#ifndef SIZE_MAX
		#define SIZE_MAX ~(size_t)0
	#endif
	#define INVALID_INDEX ~(unsigned char)0

	#define PAGE_SIZE		get_pagesize();

	#define TINY_MAX		512
	#define TINY_BLOCKS		128
	#define TINY_PAGES		(TINY_BLOCKS  * TINY_MAX) / PAGE_SIZE
	#define TINY_SIZE		PAGE_SIZE * TINY_PAGES
	#define BITMAP_WORDS	4

	#define SMALL_MAX		4096
	#define SMALL_BLOCKS	128
	#define SMALL_PAGES		(SMALL_BLOCKS  * SMALL_MAX) / PAGE_SIZE
	#define SMALL_SIZE		PAGE_SIZE * SMALL_PAGES

	#define FREELIST_SIZE	32	// MAX = INVALID_INDEX

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

		typedef struct s_options {
			int	MXFAST;								// 
			int	TRIM_THRESHOLD;						// 
			int	TOP_PAD;							// 
			int	MMAP_THRESHOLD;						// 
			int	MMAP_MAX;							// 
			int	CHECK_ACTION;						// 
			int	PERTURB;							// 
			int	ARENA_TEST;							// 
			int	ARENA_MAX;							// Hard limit for arenas based on CPU count
		} t_options;

		typedef pthread_mutex_t	mtx_t;				// Alias

		typedef struct s_bin t_bin;
		typedef struct s_bin {
			void	*ptr;							// Pointer to the free block
			t_bin	*next;							// Pointer to next free block

		} t_bin;

		typedef struct s_zone t_zone;
		typedef struct s_zone {
			size_t		size;						// Total allocated size of the zone
			size_t		used;						// Memory used in the zone
			size_t		free;						// Memory available for allocation in the zone
			size_t		blocks;						// Number of blocks (TINY_BLOCKS or SMALL_BLOCKS. 0 for LARGE zones)
			size_t		block_size;					// Size of each block (TINY_MAX or SMALL_MAX. Same as size for LARGE zones)
			uint64_t	bitmap[BITMAP_WORDS];		// Bitmap for 16-byte blocks (used only in TINY zones)
			t_zone		*next;						// Pointer to next zone
		} t_zone;
		
		typedef struct s_arena t_arena;
		typedef struct s_arena {
			int			id;             			// Arena ID			(necesario?)
			int			active;         			// Active status	(para que se usaria?)
			size_t		used;	    				// Memory used in the arena
			t_bin		fastbin[10];				// (16-160 bytes)										Arrays de listas simples (LIFO)
			t_bin		smallbin[31];				// (176-512 bytes para TINY, 513-4096 para SMALL)		Doblemente enlazadas. Tamaños fijos (FIFO)
			t_bin		largebin[10];				// ???
			t_bin		unsortedbin[10];			// ???
			t_zone		*zones[3];      			// Memory zones (TINY, SMALL, LARGE)
			mtx_t		mutex;          			// Mutex for thread safety in the current arena
			t_arena		*next;          			// Pointer to next arena
		} t_arena;

		typedef struct s_arena_manager {
			int			initialized;				// 
			int			first_alloc;				// 
			int			cpu_count;					// Number of CPUs available
			int			arena_curr;					// Current number of arenas created and active
			mtx_t		mutex;						// Mutex for synchronizing access to the arenas
			t_arena		*arenas;					// Pointer to arenas
			t_options	options;					// 
		} t_arena_manager;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	extern t_arena_manager	g_arena_manager;

	size_t	get_pagesize();

	// Arena
	int		arena_initialize();
	void	arena_terminate();
	t_arena *arena_get(size_t size);
	t_arena *arena_create();
	t_arena *arena_reuse();

	void	set_bit(uint64_t *bitmap, int pos);
	void	clear_bit(uint64_t *bitmap, int pos);
	int		is_bit_set(uint64_t *bitmap, int pos);
	int		find_first_free_bit(uint64_t *bitmap, int max_bits);

	unsigned char	get_zonetype(size_t size);
	unsigned char	get_freelist_index(size_t size);
	void			print_freelist_ranges();

	// Options
	int		mallopt(int param, int value);
	void	mallopt_init();

	// Main functions
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);

	// Utils
	int	ft_strcmp(const char *s1, const char *s2);
	int	ft_strncmp(const char *s1, const char *s2, int n);
	int	ft_isdigit_s(char *str);
	int	ft_atoi(const char *str);

#pragma endregion
