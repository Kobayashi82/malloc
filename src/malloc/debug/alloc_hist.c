/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   alloc_hist.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:16:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 13:01:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Show Alloc History"

	__attribute__((visibility("default")))
	void show_alloc_history() {
		
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
