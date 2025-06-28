/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extra.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 10:28:30 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 11:54:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Includes"

	static int is_power_of_two(size_t n) {
    	return (n != 0 && (n & (n - 1)) == 0);
	}

#pragma endregion

	void* aligned_alloc(size_t alignment, size_t size) {
		if (!is_power_of_two(alignment) || size % alignment) return (NULL);

		// Tu lógica de asignación alineada
		void* ptr = your_aligned_allocation(alignment, size);
		if (ptr) register_allocation(ptr, size, alignment);

		return (ptr);
	}

	int posix_memalign(void **memptr, size_t alignment, size_t size) {
		*memptr = NULL;

		// Validaciones POSIX específicas
		if (alignment % sizeof(void*) != 0 || !is_power_of_two(alignment)) return (EINVAL);

		void* ptr = your_aligned_allocation(alignment, size);
		if (!ptr) return (ENOMEM);

		*memptr = ptr;
		register_allocation(ptr, size, alignment);

		return (0);
	}

	void* memalign(size_t alignment, size_t size) {
		if (!is_power_of_two(alignment)) return (NULL);

		// Más flexible que aligned_alloc
		void* ptr = your_aligned_allocation(alignment, size);
		if (ptr) register_allocation(ptr, size, alignment);

		return (ptr);
	}

	void* memalign(size_t alignment, size_t size) {
		if (!is_power_of_two(alignment)) return (NULL);

		// Más flexible que aligned_alloc
		void* ptr = your_aligned_allocation(alignment, size);
		if (ptr) register_allocation(ptr, size, alignment);

		return (ptr);
	}
