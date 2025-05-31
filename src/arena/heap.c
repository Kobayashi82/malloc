/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/31 13:57:51 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"
	#include "utils.h"

#pragma endregion

#pragma region "Range"

	#pragma region "Delete"

		#pragma region "Del Array"

			int range_del_array(t_range *range, t_heap *heap) {
				bool	shrink = false;
				int		result = 1;

				for (int i = 0; i < HEAPS_MAX && range->start[i] && range->end[i]; ++i) {
					if (!shrink && range->start[i] == heap && range->end[i] == heap + heap->size) shrink = true;
					if (shrink) {
						result = 0;
						int move = HEAPS_MAX - i - 1;
						if (move > 0) {
							memmove(&range->start[i], &range->start[i + 1], move * sizeof(void *));
							memmove(&range->end[i], &range->end[i + 1], move * sizeof(void *));
						}
						range->start[HEAPS_MAX - 1] = NULL;
						range->end[HEAPS_MAX - 1] = NULL;
						break;
					}
				}

				return (result);
			}

		#pragma endregion

		#pragma region "Del Range"

			int range_del(t_heap *heap) {
				if (!heap || !tcache.arena) return (1);

				int		result = 0;
				t_arena	*arena = tcache.arena;

				mutex(&g_manager.mutex, MTX_LOCK);
				mutex(&arena->mutex, MTX_LOCK);

					result += range_del_array(&arena->range, heap);
					result += range_del_array(&g_manager.range[arena->id], heap);

				mutex(&arena->mutex, MTX_UNLOCK);
				mutex(&g_manager.mutex, MTX_UNLOCK);

				return (result != 0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Add"

		#pragma region "Add Array"

			int range_add_array(t_range *range, t_heap *heap) {
				int i = 0;

				while (i < HEAPS_MAX && range->start[i] && range->end[i]) ++i;
				if (i == HEAPS_MAX) return (1);
				range->start[i] = heap;
				range->end[i] = heap + heap->size;

				return (0);
			}

		#pragma endregion

		#pragma region "Add Range"

			int range_add(t_heap *heap) {
				if (!heap || !tcache.arena) return (1);

				int		result = 0;
				t_arena	*arena = tcache.arena;
				
				mutex(&g_manager.mutex, MTX_LOCK);
				mutex(&arena->mutex, MTX_LOCK);

					if (!(result = range_add_array(&arena->range, heap))) {
						if ((result = range_add_array(&g_manager.range[arena->id], heap)))
							range_del_array(&arena->range, heap);
					}

				mutex(&arena->mutex, MTX_UNLOCK);
				mutex(&g_manager.mutex, MTX_UNLOCK);

				return (result);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
