/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   aligned_alloc.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:06:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 14:27:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Aligned Alloc"

	// __attribute__((visibility("default")))
	// void *aligned_alloc(size_t alignment, size_t size) {
	// 	ensure_init();

	// 	// Validar parámetros
	// 	if (!is_power_of_two(alignment) || size % alignment) return (NULL);
		
	// 	void *ptr = NULL;

	// 	// Tu lógica de asignación alineada
	// 	// ptr = your_aligned_allocation(alignment, size);
	// 	// if (ptr) register_allocation(ptr, size, alignment);

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
