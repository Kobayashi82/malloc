/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/05 14:03:14 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Link Chunk"

	int link_chunk(t_chunk *chunk, size_t size, int type, t_arena *arena, t_heap *heap) {
		if (!chunk || !size || !arena || !heap || chunk->size & TOP_CHUNK) return (1);

		heap->free_chunks++;

		if (!HAS_POISON(GET_PTR(chunk))) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
			if (print_error())		aprintf(2, 0, "Memory corrupted\n");
			return (abort_now());
		}

		switch (type) {
			case FASTBIN: {
				int index = (size / NORMAL_STEP) - 1;
				if ((size_t)index >= (g_manager.options.MXFAST + sizeof(t_chunk)) / NORMAL_STEP) return (1);

				SET_FD(chunk, arena->fastbin[index]);
				arena->fastbin[index] = chunk;

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk added to FastBin\n", chunk);

				return (0);
			}
			case SMALLBIN: {
				int index = (size - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP;
				if ((size_t)index >= ((SMALLBIN_MAX - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP) + 1) return (1);

				t_chunk *bin_chunk = arena->smallbin[index];
				if (bin_chunk) {
					t_chunk *prev = GET_BK(bin_chunk);
					SET_FD(prev, chunk);
					SET_FD(chunk, bin_chunk);
					SET_BK(bin_chunk, chunk);
					SET_BK(chunk, prev);
				} else {
					SET_FD(chunk, chunk);
					SET_BK(chunk, chunk);
					arena->smallbin[index] = chunk;
				}

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					void *BK = GET_BK(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_BK(chunk, BK);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk added to Smallbin\n", chunk);

				return (0);
			}
			case UNSORTEDBIN: {
				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk added to UnsortedBin\n", chunk);

				return (0);
			}
			case LARGEBIN: {
				int index = (size - (LARGEBIN_MIN)) / LARGEBIN_STEP;
				if ((size_t)index >= ((LARGEBIN_MAX - (LARGEBIN_MIN)) / LARGEBIN_STEP) + 1) return (1);

				SET_FD(chunk, arena->largebin[index]);
				arena->largebin[index] = chunk;

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Chunk added to Largebin\n", chunk);

				return (0);
			}
		}

		return (1);
	}

#pragma endregion

#pragma region "Unlink Chunk"

	int unlink_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (1);

		if (heap->free_chunks > 0) heap->free_chunks--;

		if (!HAS_POISON(GET_PTR(chunk))) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
			if (print_error())		aprintf(2, 0, "Memory corrupted\n");
			mutex(&g_manager.mutex, MTX_UNLOCK);
			mutex(&arena->mutex, MTX_UNLOCK);
			show_alloc_mem_ex(GET_PTR(chunk), 0, 0);
			return (abort_now());
		}

		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		if (chunk_size <= (size_t)(g_manager.options.MXFAST + sizeof(t_chunk))) {
			int index = (chunk_size / NORMAL_STEP) - 1;
			if ((size_t)index >= (g_manager.options.MXFAST + sizeof(t_chunk)) / NORMAL_STEP) return (1);

			void **curr = &arena->fastbin[index];
			while (*curr) {
				if (*curr == chunk) {
					*curr = GET_FD(chunk);
					return (0);
				}
				curr = &(GET_FD(*curr));
			}
		} else if (chunk_size <= SMALLBIN_MAX) {
			int index = (chunk_size - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP;
			if ((size_t)index >= ((SMALLBIN_MAX - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP) + 1) return (1);

			void **curr = &arena->smallbin[index];
			while (*curr) {
				if (*curr == chunk) {
					t_chunk *next = GET_FD(*curr);
					t_chunk *prev = GET_BK(*curr);
					if (next != *curr) *curr = next; else arena->smallbin[index] = NULL;
					if (prev && prev != *curr) SET_FD(prev, next);
					if (next && next != *curr) SET_BK(next, prev);
					return (0);
				}
				curr = &(GET_FD(*curr));
			}
		} else if (chunk_size <= LARGEBIN_MAX) {
			int index = (chunk_size - (LARGEBIN_MIN)) / LARGEBIN_STEP;
			if ((size_t)index >= ((LARGEBIN_MAX - (LARGEBIN_MIN)) / LARGEBIN_STEP) + 1) return (1);

			void **curr = &arena->smallbin[index];
			while (*curr) {
				if (*curr == chunk) {
					t_chunk *next = GET_FD(*curr);
					t_chunk *prev = GET_BK(*curr);
					if (next != *curr) *curr = next; else arena->smallbin[index] = NULL;
					if (prev && prev != *curr) SET_FD(prev, next);
					if (next && next != *curr) SET_BK(next, prev);
					return (0);
				}
				curr = &(GET_FD(*curr));
			}
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

#pragma region "Find"

	#pragma region "FastBin"

		// fastbin array = (MXFAST + sizeof(t_chunk)) / NORMAL_STEP
		// fastbin index = (size / NORMAL_STEP) - 1

		void *find_in_fastbin(t_arena *arena, size_t size) {
			if (!arena || !size) return (NULL);

			void *ptr = NULL;

			int index = (size / NORMAL_STEP) - 1;
			if ((size_t)index >= (g_manager.options.MXFAST + sizeof(t_chunk)) / NORMAL_STEP) return (ptr);

			if (arena->fastbin[index]) {
				t_chunk *chunk = (t_chunk *)arena->fastbin[index];
				t_chunk *next = GET_NEXT(chunk);
				next->size |= PREV_INUSE;
				ptr = GET_PTR(chunk);
				
				t_heap *heap = heap_find(arena, ptr);
				if (heap && heap->active) {				
					heap->free -= (GET_SIZE(chunk) + sizeof(t_chunk));				
					unlink_chunk(chunk, arena, heap);
					
					if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Fastbin match for size %d bytes\n", ptr, size);
				} else ptr = NULL;
			}

			return (ptr);
		}

	#pragma endregion

	#pragma region "SmallBin"

		// smallbin array = ((SMALLBIN_MAX - (MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP) + 1
		// smallbin index = (size - (MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP

		 void *find_in_smallbin(t_arena *arena, size_t size) {
			if (!arena || !size) return (NULL);

			void *ptr = NULL;

			int index = (size - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP;
			if ((size_t)index >= ((SMALLBIN_MAX - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP)) / NORMAL_STEP) + 1) return (ptr);

			// aprintf(2, 0 , "%d -%d\n", index, ((SMALLBIN_MAX - (g_manager.options.MXFAST + sizeof(t_chunk) + NORMAL_STEP))));

			if (arena->smallbin[index]) {
				t_chunk *chunk = (t_chunk *)arena->smallbin[index];
				t_chunk *next = GET_NEXT(chunk);
				next->size |= PREV_INUSE;
				ptr = GET_PTR(chunk);
				
				t_heap *heap = heap_find(arena, ptr);
				if (heap && heap->active) {
					heap->free -= (GET_SIZE(chunk) + sizeof(t_chunk));
					unlink_chunk(chunk, arena, heap);
					
					if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Smallbin match for size %d bytes\n", ptr, size);
				} else ptr = NULL;
			}

			return (ptr);
		}

	#pragma endregion

	#pragma region "LargeBin"

		// largebin array = ((LARGEBIN_MAX - (LARGEBIN_MIN)) / LARGEBIN_STEP) + 1
		// largebin index = (size - (LARGEBIN_MIN)) / LARGEBIN_STEP

		void *find_in_largebin(t_arena *arena, size_t size) {
			if (!arena || !size) return (NULL);

			void *ptr = NULL;

			int index = (size - (LARGEBIN_MIN)) / LARGEBIN_STEP;
			if ((size_t)index >= ((LARGEBIN_MAX - (LARGEBIN_MIN)) / LARGEBIN_STEP) + 1) return (ptr);

			if (arena->largebin[index]) {
				t_chunk *chunk = (t_chunk *)arena->largebin[index];
				t_chunk *next = GET_NEXT(chunk);
				next->size |= PREV_INUSE;
				ptr = GET_PTR(chunk);

				t_heap *heap = heap_find(arena, ptr);
				if (heap && heap->active) {
					heap->free -= (GET_SIZE(chunk) + sizeof(t_chunk));
					unlink_chunk(chunk, arena, heap);
					
					if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Largebin match for size %d bytes\n", ptr, size);
				} else ptr = NULL;
			}

			return (ptr);
		}

	#pragma endregion

#pragma endregion

#pragma region "Find in Bin"

	void *find_memory(t_arena *arena, size_t size) {
		if (!arena || !size) return (NULL);

		if (ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK) return (heap_create(arena, LARGE, size, 0));

		void *ptr = NULL;

		size = ALIGN(size + sizeof(t_chunk));
		if (g_manager.options.MXFAST && size <= (size_t)g_manager.options.MXFAST + sizeof(t_chunk)) ptr = find_in_fastbin(arena, size);

		if (!ptr && size <= SMALLBIN_MAX) ptr = find_in_smallbin(arena, size);
		// if (!ptr) ptr = find_in_unsortedbin(arena, size);
		if (!ptr && size <= LARGEBIN_MAX) ptr = find_in_largebin(arena, size);
		// if (!ptr && fastbin no vacio, repite
		// if (!ptr && size <= 4096) ptr = find_in_largebin(arena, size);

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
