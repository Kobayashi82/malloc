/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   posix_memalign.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 20:35:27 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Posix Memalign"

	__attribute__((visibility("default")))
	int posix_memalign(void **memptr, size_t alignment, size_t size) {
		ensure_init();

		void **validate_ptr = memptr;
		if (!validate_ptr || alignment < sizeof(void *) || !is_power_of_two(alignment)) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR]  Failed to allocated %u bytes (EINVAL)\n", size);
			return (EINVAL);
		}

		void	*ptr = NULL;

		if (!size) {
			mutex(&g_manager.mutex, MTX_LOCK);

				size_t aligned_offset = (g_manager.alloc_zero_counter * alignment);
				g_manager.alloc_zero_counter++;
				
			mutex(&g_manager.mutex, MTX_UNLOCK);

			ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
			if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [POSIX_MEMALIGN] Allocated %u bytes\n", ptr, size);
			else if (!ptr) return (ENOMEM);

			*memptr = ptr;
			return (0);
		}

		ptr = allocate_aligned("POSIX_MEMALIGN", alignment, size);

		if (ptr && print_log(0))		aprintf(g_manager.options.fd_out, 1, "%p\t [POSIX_MEMALIGN] Allocated %u bytes\n", ptr, size);
		if (!ptr && print_log(1))		aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

		if (!ptr) return (ENOMEM);
		*memptr = ptr;

		return (0);
	}

#pragma endregion

#pragma region "Information"

	// Allocates aligned memory in a POSIX‑portable way.
	//
	//   int posix_memalign(void **memptr, size_t alignment, size_t size);
	//
	//   memptr    – a pointer to the memory block.
	//   alignment – the required alignment in bytes (must be a power of two and a multiple of sizeof(void *)).
	//   size      – the size of the memory block to allocate, in bytes.
	//
	//   • On success: returns 0 and *memptr receives the allocated memory block.
	//   • On failure: leaves *memptr unchanged and returns an error code:
	//       – EINVAL: alignment not valid.
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • When failure occurs, *memptr remains unspecified, and you should not free() it.
	//   • If alignment or size is invalid, errno is not set; check the return code for errors.

#pragma endregion
