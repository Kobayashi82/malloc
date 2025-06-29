/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 23:00:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Malloc"

	__attribute__((visibility("default")))
	void *malloc(size_t size) {
		ensure_init();

		t_arena	*arena;
		void	*ptr = NULL;

		if (!size) {
			mutex(&g_manager.mutex, MTX_LOCK);

				size_t aligned_offset = (g_manager.zero_malloc_counter * ALIGNMENT);
				g_manager.zero_malloc_counter++;
				
			mutex(&g_manager.mutex, MTX_UNLOCK);

			ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
			if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [MALLOC] Allocated %u bytes\n", ptr, size);
			return (ptr);
		}

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		mutex(&arena->mutex, MTX_LOCK);

			if (ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK)	ptr = heap_create(arena, LARGE, size);
			else												ptr = find_memory(arena, size);

			if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [MALLOC] Allocated %u bytes\n", ptr, size);
			else if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

			if (ptr) SET_MAGIC(ptr);

		mutex(&arena->mutex, MTX_UNLOCK);

		return (ptr);
	}

#pragma endregion

#pragma region "Information"

	// Allocates memory.
	//
	//   void *malloc(size_t size);
	//
	//   size  – the number of bytes to allocate.
	//
	//   • On success: returns a pointer aligned for the requested size.
	//   • On failure: returns NULL and sets errno to:
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • If size == 0:
	//       – returns a unique pointer that can be freed.

#pragma endregion
