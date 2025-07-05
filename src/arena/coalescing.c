/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coalescing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 11:00:49 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/05 16:41:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Coalescing Neighbours"

	t_chunk *coalescing_neighbours(t_chunk *chunk, t_arena *arena, t_heap *heap) {
		if (!chunk || !arena || !heap) return (chunk);

		t_chunk *chunk_prev = NULL;
		t_chunk *chunk_next = NULL;
		t_chunk *chunk_final = NULL;

		if (print_log(2))	aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Coalescing adjacent chunks\n", chunk);

		// Coalescing Left (If not USED)
		if (!(chunk->size & PREV_INUSE)) {
			chunk_prev = GET_PREV(chunk);
			unlink_chunk(chunk_prev, arena, heap);
			chunk_prev->size = (chunk_prev->size & (HEAP_TYPE | PREV_INUSE)) | (GET_PREV_SIZE(chunk) + GET_SIZE(chunk) + sizeof(t_chunk));
			chunk_next = GET_NEXT(chunk_prev);
			SET_PREV_SIZE(chunk_next, GET_SIZE(chunk_prev));
			chunk_final = chunk_prev;
		}

		if (!chunk_final) chunk_final = chunk;

		// Coalescing Right (if not USED)
		chunk_next = GET_NEXT(chunk_final);
		if (chunk_next->size & TOP_CHUNK) {
			if (!HAS_MAGIC(GET_PTR(chunk_next))) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Memory corrupted\n", GET_PTR(chunk));
				if (print_error())		aprintf(2, 0, "Memory corrupted\n");
				abort_now(); return (chunk_final);
			}
			chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | TOP_CHUNK | (GET_SIZE(chunk_next) + GET_SIZE(chunk_final) + sizeof(t_chunk));
			heap->top_chunk = chunk_final;
			SET_MAGIC(GET_PTR(chunk_final));
		} else {
			t_chunk *chunk_next_next = GET_NEXT(chunk_next);
			if (!(chunk_next_next->size & PREV_INUSE)) {
				unlink_chunk(chunk_next, arena, heap);
				SET_PREV_SIZE(chunk_next_next, GET_SIZE(chunk_final) + GET_SIZE(chunk_next) + sizeof(t_chunk));
				chunk_final->size = (chunk_final->size & (HEAP_TYPE | PREV_INUSE)) | GET_PREV_SIZE(chunk_next_next);
			}
		}

		if (!(chunk_final->size & TOP_CHUNK)) link_chunk(chunk_final, arena, heap);

		return (chunk_final);
	}

#pragma endregion
