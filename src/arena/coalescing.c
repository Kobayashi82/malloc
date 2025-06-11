/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coalescing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 11:00:49 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/11 11:14:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion


// Fusionar con top chunk ?

t_chunk *coalescing(t_chunk *chunk, t_arena *arena, t_heap *heap) {
	if (!chunk || !arena || !heap) return (chunk);

	t_chunk *chunk_prev = NULL;
	t_chunk *chunk_next = NULL;
	t_chunk *chunk_final = NULL;

	// Coalescing Left (If not USED and FASTBIN)
	if (!(chunk->size & PREV_INUSE) && chunk->prev_size + sizeof(t_chunk) > (t_chunk_int)g_manager.options.MXFAST) {
		chunk_prev = GET_PREV(chunk);
		// unlink_chunk(chunk_prev);
		chunk_prev->size = (chunk_prev->size & (HEAP_TYPE | PREV_INUSE)) | (chunk->prev_size + GET_SIZE(chunk) + sizeof(t_chunk));
		chunk_next = GET_NEXT(chunk_prev);
		chunk_next->prev_size = GET_SIZE(chunk_prev);
		chunk_final = chunk_prev;
	}
	if (!chunk_final) chunk_final = chunk;

	// Coalescing Right (if not USED and FASTBIN)
	chunk_next = GET_NEXT(chunk);
	if (chunk_next->size & TOP_CHUNK) {
		// unlink_chunk(chunk_next);
		chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | TOP_CHUNK | (GET_SIZE(chunk_next) + GET_SIZE(chunk_final) + sizeof(t_chunk));
		heap->top_chunk = chunk_final;
	} else {
		t_chunk *chunk_next_next = GET_NEXT(chunk_next);
		if (!(chunk_next_next->size & PREV_INUSE) && chunk_next_next->prev_size + sizeof(t_chunk) > (t_chunk_int)g_manager.options.MXFAST) {
			// unlink_chunk(chunk_next);
			chunk_next_next->prev_size = GET_SIZE(chunk_final) + GET_SIZE(chunk_next) + sizeof(t_chunk);
			chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | chunk_next_next->prev_size;
		}
	}

	return (chunk_final);
}

// MINSIZE = 32 bits = 16 y 64 bits = 24

// Free
//
// Si fue mmap'd → munmap
// Fastbin (pequeños, sin coalescing)
// Otros chunks → coalescing (left and right) + unsortedbin
//
// Free Heap
//
// contador de frees en heap (cuando vacio)
// En free a nivel de arena subir contador en heaps (cuando vacio)
// resetear contador si malloc en el heap
//
// Malloc
//
// Si large → mmap?
// Fastbin exact match
// Smallbin exact match
// Si large → consolidar fastbins a unsortedbin
// Procesar unsortedbin → small/large bins (coalescing)
// Si large → buscar en largebin y superiores (split si > MINSIZE)
// Si quedan fastbins → consolidar y repetir
// Split top chunk
//