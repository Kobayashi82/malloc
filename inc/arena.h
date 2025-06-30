/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:42:37 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 10:04:01 by vzurera-         ###   ########.fr       */
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
	t_heap	*heap_find(t_arena *arena, void *ptr);
	void	*heap_create(t_arena *arena, int type, size_t size);
	int		heap_destroy(t_heap *heap);

	// Coalescing
	int		link_chunk(t_chunk *chunk, size_t size, int type, t_arena *arena);
	int		unlink_chunk(t_chunk *chunk, t_arena *arena);
	t_chunk	*coalescing(t_chunk *chunk, t_arena *arena, t_heap *heap);

	// Bin
	void	*find_memory(t_arena *arena, size_t size);

	// Allocate
	void	*allocate_aligned(char *source, size_t alignment, size_t size);
	void	*allocate_zero(char *source);
	void	*allocate(char *source, size_t size, unsigned char perturb);

#pragma endregion
