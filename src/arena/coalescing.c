/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coalescing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 11:00:49 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/05 17:00:26 by vzurera-         ###   ########.fr       */
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
