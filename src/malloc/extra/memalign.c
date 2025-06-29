/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   memalign.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 14:27:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Memalign"

	// __attribute__((visibility("default")))
	// void *memalign(size_t alignment, size_t size) {
	// 	ensure_init();

	// 	if (!is_power_of_two(alignment)) return (NULL);

	// 	void *ptr = NULL;

	// 	(void) size;
	// 	// Más flexible que aligned_alloc
	// 	// ptr = your_aligned_allocation(alignment, size);
	// 	// if (ptr) register_allocation(ptr, size, alignment);

	// 	return (ptr);
	// }

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
