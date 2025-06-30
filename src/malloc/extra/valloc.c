/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   valloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 22:43:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 10:33:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Memalign"

	// __attribute__((visibility("default")))
	// void *valloc(size_t size) {
	// 	ensure_init();

	// 	t_arena	*arena;
	// 	void	*ptr = NULL;

	// 	if (!size) {
	// 		mutex(&g_manager.mutex, MTX_LOCK);

	// 			size_t aligned_offset = (g_manager.zero_malloc_counter * PAGE_SIZE);
	// 			g_manager.zero_malloc_counter++;
				
	// 		mutex(&g_manager.mutex, MTX_UNLOCK);

	// 		ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
	// 		if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [VALLOC] Allocated %u bytes\n", ptr, size);
	// 		else if (!ptr) errno = ENOMEM;

	// 		return (ptr);
	// 	}

	// 	if (!tcache) {
	// 		arena = arena_get();
	// 		tcache = arena;
	// 		if (!arena) {
	// 			if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to assign arena\n");
	// 			return (NULL);
	// 		}
	// 	} else arena = tcache;

	// 	mutex(&arena->mutex, MTX_LOCK);

	// 		// ptr = allocate_aligned(PAGE_SIZE, size);
			
	// 		if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [VALLOC] Allocated %u bytes\n", ptr, size);
	// 		else if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

	// 		if (ptr) SET_MAGIC(ptr);
	// 		else errno = ENOMEM;

	// 	mutex(&arena->mutex, MTX_UNLOCK);

	// 	return (ptr);
	// }

#pragma endregion

#pragma region "Information"

	// Allocates a page‑aligned block of memory.
	//
	//   void *valloc(size_t size);
	//
	//   size – the size of the memory block to allocate, in bytes.
	//
	//   • On success: returns a pointer aligned to the system page size.
	//   • On failure: returns NULL and sets errno to:
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • valloc() is non‑standard and obsolete; prefer posix_memalign() or aligned_alloc() for portable code.

#pragma endregion
