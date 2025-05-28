/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:12:35 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/28 14:01:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

	#include <stdint.h>

	#define TINY_MAX		512
	#define TINY_BLOCKS		128
	#define TINY_PAGES		(TINY_BLOCKS  * TINY_MAX) / PAGE_SIZE
	#define TINY_SIZE		PAGE_SIZE * TINY_PAGES
	#define BITMAP_WORDS	4

	#define SMALL_MAX		4096
	#define SMALL_BLOCKS	128
	#define SMALL_PAGES		(SMALL_BLOCKS  * SMALL_MAX) / PAGE_SIZE
	#define SMALL_SIZE		PAGE_SIZE * SMALL_PAGES

	
	typedef enum se_heaptype { TINY, SMALL, LARGE } e_heaptype;

typedef struct s_heap t_heap;
typedef struct s_heap {
	size_t		size;						// Total allocated size of the heap
	size_t		used;						// Memory used in the heap
	size_t		free;						// Memory available for allocation in the heap
	size_t		blocks;						// Number of blocks (TINY_BLOCKS or SMALL_BLOCKS. 0 for LARGE heaps)
	size_t		block_size;					// Size of each block (TINY_MAX or SMALL_MAX. Same as size for LARGE heaps)
	uint64_t	bitmap[BITMAP_WORDS];		// Bitmap for 16-byte blocks (used only in TINY heaps)
	t_heap		*next;						// Pointer to next zone
} t_heap;