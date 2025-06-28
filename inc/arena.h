/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:42:37 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 13:03:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "internal.h"

#pragma endregion

#pragma region "Methods"

	// Arena
	t_arena *arena_get();

	// Heap
	t_heap	*heap_find(t_arena *arena, void *ptr);
	void	*heap_create(t_arena *arena, int type, size_t size);
	int		heap_destroy(t_arena *arena, void *ptr, int type, size_t size);

	// Coalescing
	int		link_chunk(t_chunk *chunk, size_t size, int type, t_arena *arena);
	int		unlink_chunk(t_chunk *chunk, t_arena *arena);
	t_chunk	*coalescing(t_chunk *chunk, t_arena *arena, t_heap *heap);

	// Bin
	void	*find_memory(t_arena *arena, size_t size);

	// Main
	void	realfree(void *ptr);
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);
	void	*calloc(size_t nmemb, size_t size);

	void	*reallocarray(void *ptr, size_t nmemb, size_t size);
	size_t	malloc_usable_size(void *ptr);

#pragma endregion
