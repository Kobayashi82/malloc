/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memalign.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/04 13:25:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Memalign"

	__attribute__((visibility("default")))
	void *memalign(size_t alignment, size_t size) {
		ensure_init();

		if (alignment < sizeof(void *) || !is_power_of_two(alignment)) {
			if (print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR]  Failed to allocated %u bytes (EINVAL)\n", size);
			errno = EINVAL; return (NULL);
		}

		void	*ptr = NULL;

		if (!size) {
			mutex(&g_manager.mutex, MTX_LOCK);

				size_t aligned_offset = (g_manager.alloc_zero_counter * alignment);
				g_manager.alloc_zero_counter++;
				
			mutex(&g_manager.mutex, MTX_UNLOCK);

			ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
			if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [MEMALIGN] Allocated %u bytes\n", ptr, size);
			else if (!ptr) errno = ENOMEM;

			return (ptr);
		}

		ptr = allocate_aligned("MEMALIGN", alignment, size);

		return (ptr);
	}

#pragma endregion

#pragma region "Information"

	// Allocates a block with a specified alignment.
	//
	//   void *memalign(size_t alignment, size_t size);
	//
	//   alignment – alignment in bytes (must be a power of two and a multiple of sizeof(void *)).
	//   size      – the size of the block to allocate, in bytes.
	//
	//   • On success: returns a pointer aligned to alignment bytes.
	//   • On failure: returns NULL and sets errno to:
	//       – EINVAL: alignment not valid.
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • memalign() is non‑standard; prefer posix_memalign() or aligned_alloc() for portable code.

#pragma endregion
