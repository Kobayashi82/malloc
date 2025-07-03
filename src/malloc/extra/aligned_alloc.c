/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   aligned_alloc.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 14:33:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Aligned Alloc"

	__attribute__((visibility("default")))
	void *aligned_alloc(size_t alignment, size_t size) {
		ensure_init();

		if (alignment < sizeof(void *) || !is_power_of_two(alignment) || size % alignment) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR]  Failed to allocated %u bytes (EINVAL)\n", size);
			errno = EINVAL; return (NULL);
		}

		void *ptr = NULL;

		if (!size) {
			mutex(&g_manager.mutex, MTX_LOCK);

				size_t aligned_offset = (g_manager.alloc_zero_counter * alignment);
				g_manager.alloc_zero_counter++;
				
			mutex(&g_manager.mutex, MTX_UNLOCK);

			ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
			if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [ALIGNED_ALLOC] Allocated %u bytes\n", ptr, size);
			else if (!ptr) errno = ENOMEM;

			return (ptr);
		}

		ptr = allocate_aligned("ALIGNED_ALLOC", alignment, size);
		
		if (ptr && print_log(0))		aprintf(g_manager.options.fd_out, 1, "%p\t [ALIGNED_ALLOC] Allocated %u bytes\n", ptr, size);
		if (!ptr && print_log(1))		aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

		if (ptr) SET_MAGIC(ptr);
		else errno = ENOMEM;

		return (ptr);
	}

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
