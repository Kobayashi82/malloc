/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/05 16:35:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Link Chunk"

	int link_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap || chunk->size & TOP_CHUNK) return (1);
		heap->free_chunks++;
		
		SET_FD(chunk, NULL);

		if (!HAS_POISON(GET_PTR(chunk))) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted (link)\n", GET_PTR(chunk));
			if (print_error())		aprintf(2, 0, "Memory corrupted\n");
			return (abort_now());
		}

		size_t size = GET_SIZE(chunk) + sizeof(t_chunk);
		int index = (size / ALIGNMENT) - 1;
		if ((size_t)index >= (SMALL_CHUNK + sizeof(t_chunk)) / ALIGNMENT) return (1);
		
		SET_FD(chunk, arena->bins[index]);
		arena->bins[index] = chunk;

		if (g_manager.options.PERTURB) {
			void *FD = GET_FD(chunk);
			uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
			ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
			SET_FD(chunk, FD);
			SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
		}

		if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk added to Bin\n", chunk);

		return (0);
	}

#pragma endregion

#pragma region "Unlink Chunk"

	int unlink_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (1);

		if (heap->free_chunks > 0) heap->free_chunks--;

		if (!HAS_POISON(GET_PTR(chunk))) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted (unlink)\n", GET_PTR(chunk));
			if (print_error())		aprintf(2, 0, "Memory corrupted\n");
			return (abort_now());
		}

		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		int index = (chunk_size / ALIGNMENT) - 1;
		if ((size_t)index >= SMALL_CHUNK + sizeof(t_chunk)) return (1);

		aprintf(2, 0, "Me cago en tu puta madre\n");
void **curr = &arena->bins[index];
int iterations = 0;
while (*curr && iterations < 10) {  // Límite de seguridad
    aprintf(2, 0, "Iteration %d: curr=%p, *curr=%p, chunk=%p\n", iterations, curr, *curr, chunk);
    aprintf(2, 0, "GET_FD(*curr)=%p\n", GET_FD(*curr));
    
    if (*curr == chunk) {
        *curr = GET_FD(chunk);
        aprintf(2, 0, "Found and removed chunk\n");
        return (0);
    }
    
    // Verificar que el siguiente puntero no es NULL antes de avanzar
    void *next_fd = GET_FD(*curr);
    if (!next_fd) {
        aprintf(2, 0, "Next FD is NULL, breaking\n");
        break;
    }
    
    curr = (void**)((char*)*curr + sizeof(t_chunk));
    iterations++;
}
aprintf(2, 0, "Chunk not found after %d iterations\n", iterations);
		aprintf(2, 0, "Cabron\n");
		if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk removed from Bin\n", chunk);

		return (0);
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

	// array = (MXFAST + sizeof(t_chunk)) / NORMAL_STEP
	// index = (size / NORMAL_STEP) - 1

	void *find_in_bin(t_arena *arena, size_t size) {
		if (!arena || !size) return (NULL);

		void *ptr = NULL;

		int index = (size / ALIGNMENT) - 1;
		if ((size_t)index >= (SMALL_CHUNK + sizeof(t_chunk)) / ALIGNMENT) return (ptr);

		if (arena->bins[index]) {
			t_chunk *chunk = (t_chunk *)arena->bins[index];
			t_chunk *next = GET_NEXT(chunk);
			next->size |= PREV_INUSE;
			ptr = GET_PTR(chunk);

			t_heap *heap = heap_find(arena, ptr);
			if (heap && heap->active) {				
				heap->free -= (GET_SIZE(chunk) + sizeof(t_chunk));				
				unlink_chunk(chunk, arena, heap);
				
				if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Bin match for size %d bytes\n", ptr, size);
			} else ptr = NULL;
		}

		return (ptr);
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
