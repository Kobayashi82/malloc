/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reallocarray.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 12:37:30 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 14:35:55 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Realloc Array"

	__attribute__((visibility("default")))
	void *reallocarray(void *ptr, size_t nmemb, size_t size) {
		ensure_init();

		if (nmemb && nmemb > SIZE_MAX / size) {
			errno = ENOMEM;
			return (NULL);
		}

		return (realloc(ptr, nmemb * size));
	}

#pragma endregion

#pragma region "Information"

	// Reallocates memory for an array with overflow protection.
	//
	//   void *reallocarray(void *ptr, size_t nmemb, size_t size);
	//
	//   ptr   – pointer returned by malloc/calloc/realloc.
	//   nmemb – number of elements.
	//   size  – the size of each element, in bytes.
	//
	//   • On success: returns a pointer aligned for nmemb * size bytes (may be the same as ptr or a new location).
	//   • On failure: returns NULL, original block unchanged and sets errno to:
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • If nmemb == 0 || size == 0:
	//       – ptr != NULL → returns NULL, leaves ptr valid (not an error).
	//       – ptr == NULL → returns a unique pointer you can free (malloc(0)).

#pragma endregion
