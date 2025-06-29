/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   posix_memalign.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:05 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 14:06:27 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Posix Memalign"

	// __attribute__((visibility("default")))
	// int posix_memalign(void **memptr, size_t alignment, size_t size) {
	// 	ensure_init();

	// 	*memptr = NULL;

	// 	// Validaciones POSIX específicas
	// 	if (alignment % sizeof(void *) != 0 || !is_power_of_two(alignment)) return (EINVAL);

	// 	void *ptr = NULL;

	// 	(void) size;
	// 	// ptr = your_aligned_allocation(alignment, size);
	// 	// if (!ptr) return (ENOMEM);

	// 	*memptr = ptr;
	// 	// register_allocation(ptr, size, alignment);

	// 	return (0);
	// }

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
