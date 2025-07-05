/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/05 17:01:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Coalescing"

	t_chunk *coalescing_neighbours(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (chunk);

		t_chunk *chunk_prev = NULL;
		t_chunk *chunk_next = NULL;
		t_chunk *chunk_final = chunk;

		if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Coalescing adjacent chunks\n", chunk);

		// Coalescing Left
		if (!(chunk->size & PREV_INUSE)) {
			chunk_prev = GET_PREV(chunk);

			if (HAS_POISON(GET_PTR(chunk_prev))) {
				if (unlink_chunk(chunk_prev, arena, heap) == 0) {
					size_t new_size = GET_SIZE(chunk_prev) + GET_SIZE(chunk) + sizeof(t_chunk);
					chunk_prev->size = (chunk_prev->size & (HEAP_TYPE | PREV_INUSE)) | new_size;
					
					chunk_next = GET_NEXT(chunk_prev);
					SET_PREV_SIZE(chunk_next, new_size);
					
					chunk_final = chunk_prev;
				}
			}
		}

		// Coalescing Right
		chunk_next = GET_NEXT(chunk_final);
		
		if (chunk_next->size & TOP_CHUNK) {
			if (HAS_MAGIC(GET_PTR(chunk_next))) {
				size_t new_size = GET_SIZE(chunk_final) + GET_SIZE(chunk_next) + sizeof(t_chunk);
				chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | TOP_CHUNK | new_size;
				heap->top_chunk = chunk_final;
				SET_MAGIC(GET_PTR(chunk_final));
			} else {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted (top chunk)\n", GET_PTR(chunk_next));
				if (print_error())		aprintf(2, 0, "Memory corrupted\n");
				abort_now();
			}
		} else {
			t_chunk *chunk_next_next = GET_NEXT(chunk_next);
			if (!(chunk_next_next->size & PREV_INUSE) && HAS_POISON(GET_PTR(chunk_next))) {
				if (unlink_chunk(chunk_next, arena, heap) == 0) {
					size_t new_size = GET_SIZE(chunk_final) + GET_SIZE(chunk_next) + sizeof(t_chunk);
					chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | new_size;
					
					chunk_next_next = GET_NEXT(chunk_final);
					SET_PREV_SIZE(chunk_next_next, new_size);
				}
			}
		}

		if (!(chunk_final->size & TOP_CHUNK)) link_chunk(chunk_final, arena, heap);

		return (chunk_final);
	}

#pragma endregion

#pragma region "Link Chunk"

	int link_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap || (chunk->size & TOP_CHUNK)) return (1);
		
		heap->free_chunks++;
		
		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		int index = (chunk_size / ALIGNMENT) - 1;
		if ((size_t)index >= (SMALL_CHUNK + sizeof(t_chunk)) / ALIGNMENT) return (1);
		
		SET_FD(chunk, arena->bins[index]);
		arena->bins[index] = chunk;

		if (g_manager.options.PERTURB) {
			void *fd_backup = GET_FD(chunk);
			uint32_t prev_size_backup = GET_PREV_SIZE(GET_NEXT(chunk));
			ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
			SET_FD(chunk, fd_backup);
			SET_PREV_SIZE(GET_NEXT(chunk), prev_size_backup);
		}

		if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk added to Bin\n", chunk);

		return (0);
	}

#pragma endregion

#pragma region "Unlink Chunk"

	int unlink_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (1);

		if (heap->free_chunks > 0) heap->free_chunks--;

		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		int index = (chunk_size / ALIGNMENT) - 1;
		if ((size_t)index >= (SMALL_CHUNK + sizeof(t_chunk)) / ALIGNMENT) return (1);

		t_chunk **current = (t_chunk **)&arena->bins[index];
		
		while (*current) {
			if (*current == chunk) {
				*current = (t_chunk *)GET_FD(chunk);
				SET_FD(chunk, NULL);
				
				if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk removed from Bin\n", chunk);
				return (0);
			}
			current = (t_chunk **)((char *)*current + sizeof(t_chunk));
		}

		return (1);
	}

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

#pragma region "Find in Bin"

	void *find_in_bin(t_arena *arena, size_t size) {
		if (!arena || !size) return (NULL);

		int index = (size / ALIGNMENT) - 1;
		if ((size_t)index >= (SMALL_CHUNK + sizeof(t_chunk)) / ALIGNMENT) return (NULL);

		if (arena->bins[index]) {
			t_chunk *chunk = (t_chunk *)arena->bins[index];
			
			if (!HAS_POISON(GET_PTR(chunk))) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Corrupted chunk in bin\n", GET_PTR(chunk));
				if (print_error())		aprintf(2, 0, "Memory corrupted\n");
				return (NULL);
			}
			
			arena->bins[index] = GET_FD(chunk);
			SET_FD(chunk, NULL);
			t_chunk *next = GET_NEXT(chunk);
			next->size |= PREV_INUSE;
			
			t_heap *heap = heap_find(arena, GET_PTR(chunk));
			if (heap && heap->active) {
				heap->free -= (GET_SIZE(chunk) + sizeof(t_chunk));
				if (heap->free_chunks > 0) heap->free_chunks--;
			}
			
			if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Bin match for size %zu bytes\n", GET_PTR(chunk), size);
			
			return (GET_PTR(chunk));
		}

		return (NULL);
	}

#pragma endregion

#pragma region "Find Memory"

	void *find_memory(t_arena *arena, size_t size) {
		if (!arena || !size) return (NULL);

		if (ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK) return (heap_create(arena, LARGE, size, 0));

		void *ptr = NULL;

		size = ALIGN(size + sizeof(t_chunk));
		ptr = find_in_bin(arena, size);

		if (!ptr) {
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
		}

		return (ptr);
	}

#pragma endregion

#pragma region "Information"

	// chunk->size |= PREV_INUSE;			// Activate bit
	// chunk->size &= ~PREV_INUSE;			// Deactivate bit
	// if (chunk->size & PREV_INUSE) {		// Check bit
	// chunk->size ^= PREV_INUSE;			// Swap bit
	// chunk->size & ~15;					// Clear bits

	// - mmap (if too large)
	// - fastbin (exact match)
	// - smallbin (exact match)
	// - coalescing fastbins to unsortedbin (coalescing)
	// - process unsortedbin → small/large bins
	// - largebin (split si > MINSIZE)
	// - repeat if fastbin not empty
	// - split top chunk

#pragma endregion
