/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:12:35 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/02 14:13:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stdint.h>

#pragma endregion

#pragma region "Variables"

	#pragma region "Defines"

		#define TOP_CHUNK		0x4					// Bit 2
		#define IS_MMAPPED		0x2					// Bit 1
		#define PREV_INUSE		0x1					// Bit 0

		#define TINY_MAX		512
		#define TINY_BLOCKS		128
		#define TINY_PAGES		(TINY_BLOCKS  * TINY_MAX) / PAGE_SIZE
		#define TINY_SIZE		PAGE_SIZE * TINY_PAGES
		#define BITMAP_WORDS	4

		#define SMALL_MAX		4096
		#define SMALL_BLOCKS	128
		#define SMALL_PAGES		(SMALL_BLOCKS  * SMALL_MAX) / PAGE_SIZE
		#define SMALL_SIZE		PAGE_SIZE * SMALL_PAGES

	#pragma endregion

	#pragma region "Enumerators"
	
		typedef enum se_heaptype { TINY, SMALL, LARGE } e_heaptype;

	#pragma endregion

	#pragma region "Structures"

		typedef struct s_arena t_arena;

		typedef struct s_chunk {
			uint16_t	prev_size;					// Size of the previous chunk
			uint16_t	size;						// Size of the chunk
		} t_chunk;

		typedef struct s_lchunk {
			size_t		prev_size;					// Size of the previous chunk
			size_t		size;						// Size of the chunk
		} t_lchunk;

		typedef struct s_heap t_heap;
		typedef struct s_heap {
			void		*ptr;						// Pointer to the heap
			size_t		size;						// Size of the heap
			size_t		free;						// Memory available for allocation in the heap
			e_heaptype	type;						// Type of the heap
			t_heap		*prev;						// Pointer to the previous heap
			t_heap		*next;						// Pointer to the next heap
		} t_heap;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	t_heap	*heap_find(void *ptr, t_arena *arena);
	void	*heap_create(e_heaptype type, size_t size);
	int		heap_destroy(void *ptr, size_t size, e_heaptype type);

#pragma endregion
