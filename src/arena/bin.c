/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 13:23:30 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Top Chunk"

	#pragma region "Split"

		t_chunk *split_top_chunk(t_heap *heap, size_t size) {
			if (!heap || !heap->active || !heap->top_chunk || !size) return (NULL);

			t_chunk	*top_chunk = heap->top_chunk;	
			size_t	top_chunk_available = GET_SIZE(top_chunk);
			if (top_chunk_available < size) return (NULL);
			
			t_chunk *chunk = top_chunk;
			chunk->size = (top_chunk->size & (HEAP_TYPE | PREV_INUSE)) | (size - sizeof(t_chunk));
			chunk->size &= ~TOP_CHUNK;
			SET_POISON(GET_PTR(chunk));

			top_chunk = GET_NEXT(chunk);
			top_chunk->size = (top_chunk_available - size) | TOP_CHUNK | ((heap->type == SMALL) ? HEAP_TYPE : 0) | PREV_INUSE;
			heap->top_chunk = top_chunk;
			SET_MAGIC(GET_PTR(top_chunk));

			return (chunk);
		}

	#pragma endregion

	#pragma region "New Chunk"

		void *get_bestheap(t_arena *arena, int type, size_t size) {
			if (!arena || !size || type < TINY || type > SMALL) return (NULL);

			t_heap	*best_heap = NULL;
			float	best_usage = -1000;

			// Find first heap
			size_t			heap_size = (type == TINY) ? TINY_SIZE : SMALL_SIZE;
			t_heap_header	*heap_header = arena->heap_header;
			t_heap			*heap = NULL;

			if (!heap_header) {
				heap = (t_heap *)heap_create(arena, type, heap_size, 0);
				if (!heap) return (NULL);
				best_heap = heap;
			}

			// Find best heap
			if (!best_heap) {
				bool found = false;
				while (heap_header && !found) {
					heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
		
					for (int i = 0; i < heap_header->used; i++) {
						if (heap->active && heap->type == type) {
							size_t free_size = GET_SIZE(heap->top_chunk);
							float usage = ((float)(heap->size - heap->free) / (float)heap->size) * 100.0f;
							float frag_percent = (heap->free_chunks > 1) ? (1.0f - (1.0f / (float)heap->free_chunks)) * 100.0f : 0.0f;
							bool available = free_size >= size;

							if (available && usage > (float)g_manager.options.MIN_USAGE && frag_percent < (float)FRAG_PERCENT) { 
								best_heap = heap; found = true; break; 
							}
							if (available && usage > best_usage && frag_percent < (float)FRAG_PERCENT) { 
								best_usage = usage; best_heap = heap; 
							}
						}
						heap = (t_heap *)((char *)heap + ALIGN(sizeof(t_heap)));
					}

					heap_header = heap_header->next;
				}

				// Create heap (no best heap found)
				if (!best_heap) {
					best_heap = (t_heap *)heap_create(arena, type, heap_size, 0);
					if (!best_heap) return (NULL);
				}
			}

			return (best_heap);
		}

	#pragma endregion

#pragma endregion

#pragma region "Find"

	#pragma region "FastBin"

		void *find_in_fastbin(t_arena *arena, size_t size) {
			if (!arena || !size) return (NULL);

			void *ptr = NULL;

			int index = (size - 1) / ALIGNMENT;
			if (index >= g_manager.options.MXFAST / ALIGNMENT) return (ptr);
			if (arena->fastbin[index]) {
				t_chunk *chunk = (t_chunk *)arena->fastbin[index];
				arena->fastbin[index] = GET_FD(chunk);
				t_chunk *next = GET_NEXT(chunk);
				next->size |= PREV_INUSE;

				ptr = GET_PTR(chunk);
				if (print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Fastbin match for size %d bytes\n", ptr, size);

				t_heap *heap = heap_find(arena, GET_PTR(chunk));
				if (heap && heap->active) {
					heap->free -= (GET_SIZE(chunk) + sizeof(t_chunk));
					if (heap->free_chunks > 0) heap->free_chunks--;

					return (ptr);
				}
			}

			return (ptr);
		}

	#pragma endregion

	#pragma region "SmallBin"

		// static void *find_in_smallbin(t_arena *arena, size_t size) {
		// 	if (!arena || !size) return (NULL);

		// 	void *ptr = NULL;

		// 	return (ptr);
		// }

	#pragma endregion

#pragma endregion

#pragma region "Find in Bin"

	void *find_memory(t_arena *arena, size_t size) {
		if (!arena || !size) return (NULL);

		if (ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK) return (heap_create(arena, LARGE, size, 0));

		void *ptr = NULL;

		size = ALIGN(size + sizeof(t_chunk));
		if (size <= (size_t)g_manager.options.MXFAST) ptr = find_in_fastbin(arena, size);

		// if (!ptr && size <= MAX_SIZE_BIN) ptr = find_in_smallbin(arena, size);
		// if (!ptr) ptr = find_in_unsortedbin(arena, size);
		// if (!ptr) ptr = find_in_largebin(arena, size);
		// if (!ptr && fastbin no vacio, repite
		// if (!ptr) ptr = find_in_largebin(arena, size);

		int type = (size > TINY_CHUNK) ? SMALL : TINY;
		t_heap *heap = get_bestheap(arena, type, size);
		if (heap) {
			t_chunk	*chunk = split_top_chunk(heap, size);
			if (!chunk) {
				heap = (t_heap *)heap_create(arena, type, (type == TINY) ? TINY_SIZE : SMALL_SIZE, 0);
				if (!heap) return (ptr);
				chunk = split_top_chunk(heap, size);
				if (!chunk) return (ptr);
			}

			heap->free -= size;
			ptr = (GET_PTR(chunk));
		}

		return (ptr);
	}

#pragma endregion

#pragma region "Information"

	// cantidad_fastbins = MXFAST / ALIGNMENT
	// tamaño_del_chunk = (índice + 1) * ALIGNMENT
	// tamaño_máximo_real = cantidad_fastbins * ALIGNMENT

	// chunk->size |= PREV_INUSE;			// Activar bit
	// chunk->size &= ~PREV_INUSE;			// Desactivar bit
	// if (chunk->size & PREV_INUSE) {		// Comprobar bit
	// chunk->size ^= PREV_INUSE;			// Alternar bit
	// chunk->size & ~15;					// Limpiar bits

#pragma endregion
