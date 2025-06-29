/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 12:33:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		ensure_init();

		t_arena	*arena;
		void	*new_ptr = NULL;

		if (!ptr)	return malloc(size);				// ptr NULL is equivalent to malloc(size)
		if (!size)	return (free(ptr), NULL);			// size 0 is equivalent to free(ptr)

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		mutex(&arena->mutex, MTX_LOCK);

			// tiene que buscar en todas las arenas
			if (!heap_find(arena, ptr)) {
				mutex(&arena->mutex, MTX_UNLOCK);
				free(ptr);
				return (native_realloc(ptr, size));
			}

			// Extend chunk or malloc(size)
			// Free old chunk if apply

			if (ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK)	new_ptr = heap_create(arena, LARGE, size);
			else												new_ptr = find_memory(arena, size);

			if (!new_ptr) {
				if ( g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to re-allocated %u bytes\n", size);
				mutex(&arena->mutex, MTX_UNLOCK);
				free(ptr);
				return (NULL);
			}

			t_chunk *chunk = GET_HEAD(ptr);
			size_t copy_size = GET_SIZE(chunk);
			if (size < copy_size) copy_size = size;
			ft_memcpy(new_ptr, ptr, copy_size);

			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t[REALLOC] Memory reassigned from %p with %u bytes\n", new_ptr, ptr, size);

			if (new_ptr) SET_MAGIC(new_ptr);

		mutex(&arena->mutex, MTX_UNLOCK);

		free(ptr);
		return (new_ptr);
	}

#pragma endregion

#pragma region "Information"

	// Resizes a previously allocated memory block.
	//
	//   void *realloc(void *ptr, size_t size);
	//
	//   ptr   – pointer returned by malloc/calloc/realloc.
	//   size  – the new size of the memory block, in bytes.
	//
	//   • On success: returns a pointer aligned for the requested size (may be the same as ptr or a new location).
	//   • On failure: returns NULL, original block unchanged and sets errno to:
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • If size == 0:
	//       – ptr != NULL → returns NULL and free ptr.
	//       – ptr == NULL → behaves like malloc(size).

#pragma endregion
