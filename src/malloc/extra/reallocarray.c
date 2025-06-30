/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reallocarray.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 12:37:30 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 22:46:44 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Realloc Array"

	__attribute__((visibility("default")))
	void *reallocarray(void *ptr, size_t nmemb, size_t size) {
		ensure_init();

		if (nmemb && nmemb > SIZE_MAX / size) { errno = ENOMEM; return (NULL); }
		size = nmemb * size;
			
		if (!ptr)			return allocate("REALLOC_ARRAY", size, 0);				// ptr NULL is equivalent to malloc(size)
		if (!size)			return (free(ptr), allocate_zero("REALLOC_ARRAY"));	// size 0 is equivalent to free(ptr)
		if (!arena_find()) 	return (NULL);
		
		void	*new_ptr = NULL;
		bool	is_new = false;

		mutex(&tcache->mutex, MTX_LOCK);

			// tiene que buscar en todas las arenas
			if (!heap_find(tcache, ptr)) {
				mutex(&tcache->mutex, MTX_UNLOCK);
				return (native_realloc(ptr, size));
			}

			// Expandir chunk
			// Free old chunk if apply

		mutex(&tcache->mutex, MTX_UNLOCK);

		// Si no se puede expandir
		new_ptr = allocate("REALLOC_ARRAY", size, 0);
		if (new_ptr) {
			is_new = true;
			size_t copy_size = GET_SIZE((t_chunk *)GET_HEAD(ptr));
			ft_memcpy(new_ptr, ptr, (size < copy_size) ? size : copy_size);
		}

		if (is_new) free(ptr);
		return (new_ptr);
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
