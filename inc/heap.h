/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:12:35 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/23 16:20:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stdint.h>

#pragma endregion

#pragma region "Variables"

	#pragma region "Defines"
	
		typedef uint32_t t_chunk_int;				// Limited to 536,870,912 or flags will be overwritten. If more is needed, switch to size_t

		#define HAS_POISON(ptr)				(*(uint8_t *)ptr == POISON_BYTE)
		#define SET_POISON(ptr)				(*(uint8_t *)ptr = POISON_BYTE)
		#define CLEAN_POISON(ptr)			(*(uint8_t *)ptr = CLEAN_BYTE)
		#define IS_ALIGNED(ptr)				(((uintptr_t)GET_HEAD(ptr) & (ALIGNMENT - 1)) == 0)
		#define SET_FD(chunk, next_chunk)	(*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint8_t)) = (next_chunk))
		#define SET_BK(chunk, prev_chunk)	(*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint8_t) + sizeof(void *)) = (prev_chunk))
		#define GET_FD(chunk)				*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint8_t))
		#define GET_BK(chunk)				*(void **)((char *)(chunk) + sizeof(t_chunk) + sizeof(uint8_t) + sizeof(void *))
		#define GET_PTR(chunk)				(void *)((char *)(chunk) + sizeof(t_chunk))
		#define GET_HEAD(chunk) 			(void *)((char *)(chunk) - sizeof(t_chunk))
		#define GET_NEXT(chunk)				(t_chunk *)((char *)(chunk) + sizeof(t_chunk) + ((chunk)->size & ~7))
		#define GET_PREV(chunk)				(t_chunk *)((char *)(chunk) - ((chunk)->prev_size + sizeof(t_chunk)))
		#define GET_SIZE(chunk) 			(size_t)((chunk)->size & ~7)
		#define ALIGN(size)					(((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

		#define TOP_CHUNK					0x4		// Bit 2 (size)
		#define HEAP_TYPE					0x2		// Bit 1 (size)
		#define PREV_INUSE					0x1		// Bit 0 (size)

		#define TINY_USER					512
		#define TINY_HEADER					2 * sizeof(t_chunk_int)
		#define TINY_BLOCKS					128
		#define TINY_CHUNKS					(TINY_BLOCKS * (TINY_USER + TINY_HEADER))
		#define TINY_SIZE					((TINY_CHUNKS + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

		#define SMALL_USER					4096
		#define SMALL_HEADER				2 * sizeof(t_chunk_int)
		#define SMALL_BLOCKS				128
		#define SMALL_CHUNKS				(SMALL_BLOCKS * (SMALL_USER + SMALL_HEADER))
		#define SMALL_SIZE					((SMALL_CHUNKS + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

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

		typedef struct s_lchunk {
			size_t			prev_size;					// Size of the previous chunk
			size_t			size;						// Size of the chunk
		} t_lchunk;

		typedef struct s_heap {
			void			*ptr;						// Pointer to the heap
			size_t			size;						// Size of the heap
			size_t			free;						// Memory available for allocation in the heap
			e_heaptype		type;						// Type of the heap
			t_chunk			*top_chunk;					// Pointer to the top chunk
			struct s_heap	*prev;						// Pointer to the previous heap
			struct s_heap	*next;						// Pointer to the next heap
		} t_heap;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	t_heap	*heap_find(void *ptr, t_arena *arena);
	void	*heap_create(e_heaptype type, size_t size);
	int		heap_destroy(void *ptr, size_t size, e_heaptype type);
	int		heap_free(void *ptr, size_t size, e_heaptype type, t_heap **heap);

#pragma endregion
