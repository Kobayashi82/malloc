/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:42:37 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/26 13:44:55 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "internal.h"

#pragma endregion

#pragma region "Methods"

	// Arena
	void	arena_terminate();
	t_arena *arena_get();

	// Heap
	t_iheap	*heap_find(void *ptr, t_arena *arena);
	void	*heap_create(t_arena *arena, int type, size_t size);
	int		heap_destroy(void *ptr, size_t size, int type);
	int		heap_free(void *ptr, size_t size, int type, t_iheap *heap);

	// Coalescing
	int		link_chunk(t_chunk *chunk, size_t size, int type, t_arena *arena);
	int		unlink_chunk(t_chunk *chunk, t_arena *arena);
	t_chunk	*coalescing(t_chunk *chunk, t_arena *arena, t_iheap *heap);

	// Bin
	void	*find_memory(t_arena *arena, size_t size);

	// Main
	void	realfree(void *ptr);
	void	*realrealloc(void *ptr, size_t size);
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);
	void	*calloc(size_t nmemb, size_t size);

#pragma endregion
