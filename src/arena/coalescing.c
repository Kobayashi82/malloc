/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coalescing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 11:00:49 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 13:48:07 by vzurera-         ###   ########.fr       */
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
				int index = (size - 1) / ALIGNMENT;
				if (index >= g_manager.options.MXFAST / ALIGNMENT) return (1);

				SET_FD(chunk, arena->fastbin[index]);
				arena->fastbin[index] = chunk;

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB ^ 255, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				return (0);
			}
			case SMALLBIN:

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB ^ 255, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				return (0);
			case UNSORTEDBIN:

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB ^ 255, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				return (0);
			case LARGEBIN:
				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB ^ 255, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				return (0);
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
			return (abort_now());
		}

		size_t chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		if (chunk_size <= (size_t)g_manager.options.MXFAST) {
			int index = (chunk_size - 1) / ALIGNMENT;
			if (index >= g_manager.options.MXFAST / ALIGNMENT) return (1);

			void **curr = &arena->fastbin[index];
			while (*curr) {
				if (*curr == chunk) {
					*curr = GET_FD(chunk);
					return (0);
				}
				curr = &(GET_FD(*curr));
			}
		} else if (chunk_size <= (size_t)g_manager.options.MXFAST) { // Limite de smallbin

		} else { // buscar en unsortedbin y en largebin

		}
		// buscar chunk en bin
		// unlink

		return (1);
	}

#pragma endregion

#pragma region "Coalescing Neighbours"

	t_chunk *coalescing_neighbours(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (chunk);

		t_chunk *chunk_prev = NULL;
		t_chunk *chunk_next = NULL;
		t_chunk *chunk_final = NULL;

		// Coalescing Left (If not USED and FASTBIN)
		if (!(chunk->size & PREV_INUSE) && GET_PREV_SIZE(chunk) + sizeof(t_chunk) > (size_t)g_manager.options.MXFAST) {
			chunk_prev = GET_PREV(chunk);
			unlink_chunk(chunk_prev, arena, heap);
			chunk_prev->size = (chunk_prev->size & (HEAP_TYPE | PREV_INUSE)) | (GET_PREV_SIZE(chunk) + GET_SIZE(chunk) + sizeof(t_chunk));
			chunk_next = GET_NEXT(chunk_prev);
			SET_PREV_SIZE(chunk_next, GET_SIZE(chunk_prev));
			chunk_final = chunk_prev;
		}

		if (!chunk_final) chunk_final = chunk;

		// Coalescing Right (if not USED and FASTBIN)
		chunk_next = GET_NEXT(chunk);
		if (chunk_next->size & TOP_CHUNK) {
			if (!HAS_MAGIC(GET_PTR(chunk_next))) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
				if (print_error())		aprintf(2, 0, "Memory corrupted\n");
				abort_now(); return (chunk_final);
			}
			unlink_chunk(chunk_final, arena, heap);
			chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | TOP_CHUNK | (GET_SIZE(chunk_next) + GET_SIZE(chunk_final) + sizeof(t_chunk));
			heap->top_chunk = chunk_final;
			SET_MAGIC(GET_PTR(chunk_final));
		} else {
			t_chunk *chunk_next_next = GET_NEXT(chunk_next);
			if (!(chunk_next_next->size & PREV_INUSE) && GET_PREV_SIZE(chunk_next_next) + sizeof(t_chunk) > (size_t)g_manager.options.MXFAST) {
				unlink_chunk(chunk_next, arena, heap);
				SET_PREV_SIZE(chunk_next_next, GET_SIZE(chunk_final) + GET_SIZE(chunk_next) + sizeof(t_chunk));
				chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | GET_PREV_SIZE(chunk_next_next);

			}
		}

		return (chunk_final);
	}

#pragma endregion

#pragma region "Coalescing"

	void coalescing(t_arena *arena, t_heap *heap) {
		if (!arena || !heap) return ;

	}

#pragma endregion

#pragma region "Info"

	// MINSIZE = 32 bits = 32 y 64 bits = 64

	// Free
	//
	// Si fue mmap'd → munmap
	// Fastbin (pequeños, sin coalescing)
	// Otros chunks → coalescing (left and right) + unsortedbin

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

#pragma endregion
