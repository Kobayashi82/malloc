/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reallocarray.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 12:37:30 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 13:03:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Malloc Usable Size"

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

	// reallocarray(): reallocates memory for an array with overflow protection.
	//
	//   ptr   – existing block from malloc/calloc/realloc, or NULL
	//   nmemb – number of elements
	//   size  – size (in bytes) of each element
	//
	// How it works:
	//   • Checks if nmemb * size would overflow SIZE_MAX.
	//     – On overflow: sets errno = ENOMEM, returns NULL, leaves *ptr untouched.
	//   • Otherwise calls realloc(ptr, nmemb * size).
	//     – Success: returns a pointer aligned for nmemb * size bytes
	//       (may be the same as ptr or a new location).
	//     – Failure: returns NULL, errno = ENOMEM, original block unchanged.
	//
	// Special cases:
	//   • If nmemb == 0 || size == 0:
	//       – ptr != NULL → returns NULL, leaves ptr valid (not an error).
	//       – ptr == NULL → behaves like malloc(0) (may return NULL or a unique pointer you can free).
	//

#pragma endregion
