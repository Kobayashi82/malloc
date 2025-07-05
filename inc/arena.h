/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:42:37 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/05 15:46:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "internal.h"
	#include "malloc.h"

#pragma endregion

#pragma region "Methods"

	// Arena
	t_arena	*arena_find();
	t_arena *arena_get();

	// Heap
	int		heap_can_removed(t_arena *arena, t_heap *src_heap);
	t_heap	*heap_find(t_arena *arena, void *ptr);
	void	*heap_create(t_arena *arena, int type, size_t size, size_t alignment);
	int		heap_destroy(t_heap *heap);
	void	heap_hist_extend();
	void	heap_hist_destroy();

	// Coalescing
	int		link_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap);
	int		unlink_chunk(t_chunk *chunk, t_arena *arena, t_heap *heap);
	t_chunk	*coalescing_neighbours(t_chunk *chunk, t_arena *arena, t_heap *heap);
	void	coalescing(t_arena *arena, t_heap *heap);

	// Bin
	t_chunk	*split_top_chunk(t_heap *heap, size_t size);
	void	*get_bestheap(t_arena *arena, int type, size_t size);
	void	*find_memory(t_arena *arena, size_t size);

	// Allocate
	int		check_digit(void *ptr1, void *ptr2);
	void	*allocate_aligned(char *source, size_t alignment, size_t size);
	void	*allocate_zero(char *source);
	void	*allocate(char *source, size_t size);

#pragma endregion
