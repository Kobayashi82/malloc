/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:12:35 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/26 00:02:48 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stdint.h>

#pragma endregion

#pragma region "Variables"

	#pragma region "Defines"
	
		typedef size_t t_chunk_int;

		#define HAS_POISON(ptr)				(*(uint32_t *)ptr == POISON_BYTE)
		#define SET_POISON(ptr)				(*(uint32_t *)ptr = POISON_BYTE)
		#define CLEAN_POISON(ptr)			(*(uint32_t *)ptr = CLEAN_BYTE)
		#define IS_ALIGNED(ptr)				(((uintptr_t)GET_HEAD(ptr) & (ALIGNMENT - 1)) == 0)
		#define SET_FD(chunk, next_chunk)	(*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint32_t)) = (next_chunk))
		#define SET_BK(chunk, prev_chunk)	(*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint32_t) + sizeof(void *)) = (prev_chunk))
		#define GET_FD(chunk)				*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint32_t))
		#define GET_BK(chunk)				*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint32_t) + sizeof(void *))
		#define GET_PTR(chunk)				(void *)((char *)(chunk) + sizeof(t_chunk))
		#define GET_HEAD(chunk) 			(void *)((char *)(chunk) - sizeof(t_chunk))
		#define GET_NEXT(chunk)				(t_chunk *)((char *)(chunk) + sizeof(t_chunk) + ((chunk)->size & ~7))
		#define GET_PREV(chunk)				(t_chunk *)((char *)(chunk) - ((chunk)->prev_size + sizeof(t_chunk)))
		#define GET_SIZE(chunk) 			(size_t)((chunk)->size & ~7)
		#define ALIGN(size)					(((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

		#define TOP_CHUNK					0x4		// Bit 2 (size)
		#define HEAP_TYPE					0x2		// Bit 1 (size)
		#define PREV_INUSE					0x1		// Bit 0 (size)

		#define TINY_CHUNK					512
		#define TINY_BLOCKS					128
		#define TINY_SIZE					(((TINY_BLOCKS * TINY_CHUNK) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

		#define SMALL_CHUNK					4096
		#define SMALL_BLOCKS				128
		#define SMALL_SIZE					(((SMALL_BLOCKS * SMALL_CHUNK) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

	#pragma endregion

	#pragma region "Enumerators"
	
		typedef enum se_heaptype { TINY, SMALL, LARGE } e_heaptype;
		typedef enum se_bintype { FASTBIN, SMALLBIN, UNSORTEDBIN, LARGEBIN } e_bintype;

	#pragma endregion

	#pragma region "Structures"

		typedef struct s_arena t_arena;

		typedef struct s_chunk {
			t_chunk_int		prev_size;					// Size of the previous chunk
			t_chunk_int		size;						// Size of the chunk
		} t_chunk;

		typedef struct s_hiheap {
			uint8_t 		total;						// 60 or 50
			uint8_t 		used;
			void			*next;
		} t_hiheap;

		typedef struct s_iheap {
			void			*ptr;						// Pointer to the heap
			size_t			size;						// Size of the heap
			size_t			free;						// Memory available for allocation in the heap
			bool			active;						// Set to false when freed
			e_heaptype		type;						// Type of the heap
			t_chunk			*top_chunk;					// Pointer to the top chunk
		} t_iheap;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	t_iheap	*heap_find(void *ptr, t_arena *arena);
	void	*heap_create(t_arena *arena, e_heaptype type, size_t size);
	int		heap_destroy(void *ptr, size_t size, e_heaptype type);
	int		heap_free(void *ptr, size_t size, e_heaptype type, t_iheap *heap);

#pragma endregion
