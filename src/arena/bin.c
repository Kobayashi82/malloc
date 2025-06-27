/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/27 23:59:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Top Chunk"

	#pragma region "Split"

		static t_chunk *split_top_chunk(t_heap *heap, size_t size) {
			if (!heap || !heap->active || !heap->top_chunk || !size) return (NULL);

			t_chunk	*top_chunk = heap->top_chunk;	
			size_t	top_chunk_available = GET_SIZE(top_chunk);
			if (top_chunk_available < size) return (NULL);
			
			t_chunk *chunk = top_chunk;
			chunk->size = (top_chunk->size & (HEAP_TYPE | PREV_INUSE)) | (size - sizeof(t_chunk));
			chunk->size &= ~TOP_CHUNK;

			// static int p = 0;
			// aprintf(g_manager.options.fd_out, "split %d\n", p++);

			top_chunk = GET_NEXT(chunk);
			top_chunk->size = (top_chunk_available - size) | TOP_CHUNK | ((heap->type == SMALL) ? HEAP_TYPE : 0) | PREV_INUSE;
			heap->top_chunk = top_chunk;

			return (chunk);
		}

	#pragma endregion

	#pragma region "New Chunk"

		static void *new_chunk(t_arena *arena, int type, size_t size) {
			if (!arena || !size || type < TINY || type > SMALL) return (NULL);

			void	*ptr = NULL;
			bool	created = false;
			t_heap	*best_heap = NULL;
			float	best_usage = -1000;

			// Find first heap
			size_t			heap_size = (type == TINY) ? TINY_SIZE : SMALL_SIZE;
			t_heap_header	*heap_header = arena->heap_header;
			t_heap			*heap = NULL;

			if (!heap_header) {
				created = true;
				heap = (t_heap *)heap_create(arena, type, heap_size);
				if (!heap) return (ptr);
				best_heap = heap;
			}

			// Find best heap
			if (!best_heap) {
				bool found = false;
				while (heap_header && !found) {
					heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
		
					for (int i = 0; i < heap_header->used; i++) {
						if (heap->active && heap->type == type) {
							size_t	free_size = GET_SIZE(heap->top_chunk);
							bool	available = free_size >= size;
							int		usage = ((heap_size - free_size) * 100) / heap_size;

							if (available && usage > g_manager.options.MIN_USAGE_PERCENT) { best_heap = heap; found = true; break; }
							if (available && usage > best_usage) { best_usage = usage; best_heap = heap; }
						}
						heap++;
					}

					heap_header = heap_header->next;
				}

				// Create heap (no best heap found)
				if (!best_heap) {
					created = true;
					best_heap = (t_heap *)heap_create(arena, type, heap_size);
					if (!best_heap) return (ptr);
				}
			}

			// Split top chunk
			t_chunk	*chunk = split_top_chunk(best_heap, size);
			if (!chunk && !created) {
				best_heap = (t_heap *)heap_create(arena, type, heap_size);
				if (!best_heap) return (ptr);
				chunk = split_top_chunk(best_heap, size);
				if (!chunk) return (ptr);
			}

			best_heap->free -= size;
			ptr = (GET_PTR(chunk));

			return (ptr);
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
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t [SYSTEM] Fastbin match for size %d bytes\n", ptr, size);
				return (ptr);
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

		void *ptr = NULL;

		size = ALIGN(size + sizeof(t_chunk));

		if (size <= (size_t)g_manager.options.MXFAST) ptr = find_in_fastbin(arena, size);

		// if (!ptr && size <= MAX_SIZE_BIN) ptr = find_in_smallbin(arena, size);
		// if (!ptr) ptr = find_in_unsortedbin(arena, size);
		// if (!ptr) ptr = find_in_largebin(arena, size);
		// if (!ptr && fastbin no vacio, repite
		// if (!ptr) ptr = find_in_largebin(arena, size);

		if (!ptr) ptr = new_chunk(arena, (size > TINY_CHUNK) ? SMALL : TINY, size);

		if (ptr) arena->alloc_count++;

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
