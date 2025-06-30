/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coalescing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 11:00:49 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 11:26:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Link Chunk"

	int link_chunk(t_chunk *chunk, size_t size, int type, t_arena *arena) {
		if (!chunk || !size || !arena || chunk->size & TOP_CHUNK) return (1);

		if (!HAS_POISON(GET_PTR(chunk))) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Memory corrupted\n");
			return (abort_now());
		}

		// set perturb
		// get data on chunk (FD, prev_size y si es un chunk grande, BK y demas)
		// set perturb
		// volver a poner la data

		switch (type) {
			case FASTBIN: {
				int index = (size - 1) / ALIGNMENT;
				if (index >= g_manager.options.MXFAST / ALIGNMENT) return (1);

				SET_FD(chunk, arena->fastbin[index]);
				arena->fastbin[index] = chunk;

				if (g_manager.options.PERTURB) {
					void *FD = GET_FD(chunk);
					uint32_t PREV_SIZE = GET_PREV_SIZE(GET_NEXT(chunk));
					ft_memset(GET_PTR(chunk), g_manager.options.PERTURB, GET_SIZE(chunk));
					SET_FD(chunk, FD);
					SET_PREV_SIZE(GET_NEXT(chunk), PREV_SIZE);
				}

				return (0);
			}
			case SMALLBIN:
				return (0);
			case UNSORTEDBIN:
				return (0);
			case LARGEBIN:
				return (0);
		}

		return (1);
	}

#pragma endregion

#pragma region "Unlink Chunk"

	int unlink_chunk(t_chunk *chunk, t_arena *arena) {
		if (!chunk || !arena) return (1);

		if (!HAS_POISON(GET_PTR(chunk))) {
			if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
			else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Memory corrupted\n");
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

#pragma region "Coalescing"

	t_chunk *coalescing(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (chunk);

		t_chunk *chunk_prev = NULL;
		t_chunk *chunk_next = NULL;
		t_chunk *chunk_final = NULL;

		// Coalescing Left (If not USED and FASTBIN)
		if (!(chunk->size & PREV_INUSE) && GET_PREV_SIZE(chunk) + sizeof(t_chunk) > (size_t)g_manager.options.MXFAST) {
			chunk_prev = GET_PREV(chunk);
			unlink_chunk(chunk_prev, arena);
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
				if		(g_manager.options.DEBUG)					aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
				else if (g_manager.options.CHECK_ACTION != 2)		aprintf(2, "Memory corrupted\n");
				abort_now(); return (chunk_final);
			}
			unlink_chunk(chunk_final, arena);
			chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | TOP_CHUNK | (GET_SIZE(chunk_next) + GET_SIZE(chunk_final) + sizeof(t_chunk));
			heap->top_chunk = chunk_final;
			SET_MAGIC(GET_PTR(chunk_final));
		} else {
			t_chunk *chunk_next_next = GET_NEXT(chunk_next);
			if (!(chunk_next_next->size & PREV_INUSE) && GET_PREV_SIZE(chunk_next_next) + sizeof(t_chunk) > (size_t)g_manager.options.MXFAST) {
				unlink_chunk(chunk_next, arena);
				SET_PREV_SIZE(chunk_next_next, GET_SIZE(chunk_final) + GET_SIZE(chunk_next) + sizeof(t_chunk));
				chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | GET_PREV_SIZE(chunk_next_next);
			}
		}

		return (chunk_final);
	}

#pragma endregion

#pragma region "Info"

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

#pragma endregion
