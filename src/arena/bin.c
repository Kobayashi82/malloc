/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/04 11:29:30 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

// chunk->size |= PREV_INUSE;			// Activar bit
// chunk->size &= ~PREV_INUSE;			// Desactivar bit
// if (chunk->size & PREV_INUSE) {		// Comprobar bit
// chunk->size ^= PREV_INUSE;			// Alternar bit
// chunk->size & ~7;					// Limpiar bits

static void *find_in_fastbin(t_arena *arena, size_t size) {
	if (!arena || !size) return (NULL);
		
	void *ptr = NULL;

	int index = (size - 1) / 8;
	if (index >= M_MXFAST / 8) return (ptr);
	if (arena->fastbin[index]) {
		t_chunk *chunk = (t_chunk *)arena->fastbin[index];
		arena->fastbin[index] = GET_FD(chunk);
		t_chunk *next = GET_NEXT(chunk);
		next->size |= PREV_INUSE;

		ptr = GET_PTR(chunk);
		if (g_manager.options.DEBUG) aprintf(1, "%p\t [MALLOC] Fastbin match for size %d bytes", ptr, size);
		return (ptr);
	}

	return (ptr);
}

// static void *find_in_smallbin(t_arena *arena, size_t size) {
// 	if (!arena || !size) return (NULL);

// 	void *ptr = NULL;
// 	return (ptr);

// 	int index = (size - 1) / 8;
// 	if (index >= M_MXFAST / 8) return (ptr);
// 	if (arena->fastbin[index]) {
// 		t_chunk *chunk = (t_chunk *)arena->fastbin[index];
// 		arena->fastbin[index] = GET_FD(chunk);
// 		t_chunk *next = GET_NEXT(chunk);
// 		next->size |= PREV_INUSE;

// 		ptr = GET_PTR(chunk);
// 		if (g_manager.options.DEBUG) aprintf(1, "%p\t [MALLOC] Fastbin match for size %d bytes", ptr, size);
// 		return (ptr);
// 	}

// 	return (ptr);
// }

// La fragmentación se mide típicamente como la diferencia entre el espacio libre total
// y el bloque libre más grande. Si tienes 1000 bytes libres pero el bloque más grande
// es solo 100 bytes, tienes alta fragmentación porque no puedes satisfacer solicitudes
// grandes a pesar de tener espacio. También se puede medir por el número de bloques libres
// separados o el ratio entre espacio utilizable vs desperdiciado

// diferencia espacio libre y bloque libre mas grande
// numero bloques libres separados
// ratio espacio libre vs desperdiciado


static float get_fragmentation(t_heap *heap) {
	if (!heap) return (0);

	size_t free_space = heap->free;
	size_t largest_chunk = 500;

	return ((float)(free_space - largest_chunk) / free_space * 100);
}

static float heap_score(t_heap *heap, float usage, float frag) {
	if (!heap) return (-1000);

	const float WEIGHT_USAGE = 0.7;
	const float WEIGHT_FRAG = 0.3;

	return ((usage * WEIGHT_USAGE) - (frag * WEIGHT_FRAG));
}

static t_chunk *split_top_chunk(t_heap *heap, t_chunk_int size) {
	if (!heap || !size) return (NULL);

	t_chunk *top_chunk = heap->top_chunk;
	if (GET_SIZE(top_chunk) < size) return (NULL);
	uint16_t top_chunk_available = GET_SIZE(top_chunk);
	t_chunk_int type = top_chunk->size & HEAP_TYPE;
	top_chunk->size = (top_chunk->size & (HEAP_TYPE | PREV_INUSE)) | size;
	t_chunk *chunk = top_chunk;
	top_chunk = GET_NEXT(chunk);
	top_chunk->size = (top_chunk_available - size) | TOP_CHUNK | type | PREV_INUSE;
	top_chunk->prev_size = size;
	heap->top_chunk = top_chunk;

	return (chunk);
}

static void *new_chunk(t_arena *arena, t_chunk_int size, e_heaptype type) {
	if (!arena || !size || type < TINY || type > SMALL) return (NULL);

	void *ptr = NULL;

	size_t	heap_size = type == TINY ? TINY_SIZE : SMALL_SIZE;
	t_heap	*heap = type == TINY ? arena->tiny : arena->small;
	if (!heap) {
		if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] No heap available. Creating one\n");
		heap_create(type, heap_size);
		heap = type == TINY ? arena->tiny : arena->small;
		if (!heap) {
			if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] Heap creation failed\n");
			return (ptr);
		}
	}

	t_heap	*best_heap = NULL;
	float	best_score = -1000;
	
	while (heap) {
		bool	available = heap->free >= size + (sizeof(t_chunk) * 2);
		float	usage = ((heap_size - heap->free) * 100) / heap_size;
		float	frag = get_fragmentation(heap);

		if (available && usage > 10 && frag < 80) { 
			if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] Found heap with %d usage\n", usage);
			best_heap = heap; break;
		}
		if (available) {
			float score = heap_score(heap, usage, frag);
			if (score > best_score) { best_score = score; best_heap = heap; }
		}
		heap = heap->next;
	}

	if (!best_heap) {
		if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] No heap available. Creating one\n");
		heap_create(type, heap_size);
		best_heap = type == TINY ? arena->tiny : arena->small;
		if (!best_heap) {
			if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] Heap creation failed\n");
			return (ptr);
		}
	}

	if (best_heap) {
		if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] Best heap found\n");
		t_chunk	*chunk = split_top_chunk(best_heap, size + sizeof(t_chunk));
		if (chunk) ptr = GET_PTR(chunk);
	}
	return (ptr);
}

void *find_in_bin(t_arena *arena, size_t size) {
	if (!arena || !size) return (NULL);

	// alinear size

	void *ptr = NULL;

	if (size <= (size_t)g_manager.options.MXFAST) ptr = find_in_fastbin(arena, size);
	// if (!ptr) ptr = find_in_smallbin(arena, size);

	if (!ptr) {
		if (g_manager.options.DEBUG) aprintf(1, "\t\t   [WARN] No chunk in bins\n");
		ptr = new_chunk(arena, size, (t_chunk_int)size > TINY_USER ? SMALL : TINY);
	}

	return (ptr);
}

// cantidad_fastbins = MXFAST / 8
// tamaño_del_chunk = (índice + 1) * 8
// tamaño_máximo_real = cantidad_fastbins * 8