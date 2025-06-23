/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/23 22:43:03 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Malloc"

	__attribute__((visibility("default")))
	void *malloc(size_t size) {
		ensure_init();

		t_arena	*arena;
		void	*ptr = NULL;

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		mutex(&arena->mutex, MTX_LOCK);

			if (!size) size = 1;

			// cambiar uint8_t para que sea solo cuando es menor de 24
			if (ALIGN(size + sizeof(t_chunk) + sizeof(uint8_t)) > SMALL_USER)	ptr = heap_create(LARGE, size);
			else																ptr = find_memory(arena, size);

			if (ptr && g_manager.options.DEBUG)	aprintf(1, "%p\t [MALLOC] Allocated %d bytes\n", ptr, size);
			else if (g_manager.options.DEBUG)	aprintf(1, "\t\t  [ERROR] Failed to allocated %d bytes\n", size);

		mutex(&arena->mutex, MTX_UNLOCK);

		CLEAN_POISON(ptr);
		return (ptr);
	}

#pragma endregion
