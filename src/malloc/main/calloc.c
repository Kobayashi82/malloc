/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 12:25:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 22:46:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Calloc"

	__attribute__((visibility("default")))
	void *calloc(size_t nmemb, size_t size) {
		ensure_init();

		if (nmemb && size && nmemb > SIZE_MAX / size) return (NULL);

		return (allocate("CALLOC", nmemb * size, 0));
	}

#pragma endregion

#pragma region "Information"

	// Allocates memory for an array of objects, initializing them to zero.
	//
	//   void *calloc(size_t nmemb, size_t size);
	//
	//   nmemb – number of elements.
	//   size  – the size of each element, in bytes.
	//
	//   • On success: returns a pointer aligned for the requested size with all bytes initialized to zero.
	//   • On failure: returns NULL and sets errno to:
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • If nmemb == 0 || size == 0:
	//       – returns a unique pointer that can be freed.
	//   • Memory is zero-initialized, meaning all bits are set to 0.

#pragma endregion
