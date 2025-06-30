/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   aligned_alloc.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 10:33:04 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Aligned Alloc"

	// __attribute__((visibility("default")))
	// void *aligned_alloc(size_t alignment, size_t size) {
	// 	ensure_init();

	// 	if (alignment < sizeof(void *) || !is_power_of_two(alignment) || size % alignment) {
	// 		if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR]  Failed to allocated %u bytes\n", size);
	// 		errno = EINVAL; return (NULL);
	// 	}

	// 	t_arena	*arena;
	// 	void	*ptr = NULL;

	// 	if (!size) {
	// 		mutex(&g_manager.mutex, MTX_LOCK);

	// 			size_t aligned_offset = (g_manager.zero_malloc_counter * alignment);
	// 			g_manager.zero_malloc_counter++;
				
	// 		mutex(&g_manager.mutex, MTX_UNLOCK);

	// 		ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
	// 		if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [ALIGNED_ALLOC] Allocated %u bytes\n", ptr, size);
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

	// 		// ptr = allocate_aligned(alignment, size);
			
	// 		if (ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [ALIGNED_ALLOC] Allocated %u bytes\n", ptr, size);
	// 		else if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

	// 		if (ptr) SET_MAGIC(ptr);
	// 		else errno = ENOMEM;

	// 	mutex(&arena->mutex, MTX_UNLOCK);

	// 	return (ptr);
	// }

#pragma endregion

#pragma region "Information"

	// Allocates memory with a given alignment.
	//
	//	 void *aligned_alloc(size_t alignment, size_t size);
	//
	//   alignment – the required alignment in bytes (must be a power of two and a multiple of sizeof(void *)).
	//   size      – the size of the memory block to allocate, in bytes (must be a multiple of alignment).
	//
	//   • On success: returns a pointer aligned to the specified alignment.
	//   • On failure: returns NULL and sets errno to:
	//       – EINVAL: alignment not valid.
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • When size is not naturally a multiple of alignment, round it up before calling aligned_alloc().

#pragma endregion
