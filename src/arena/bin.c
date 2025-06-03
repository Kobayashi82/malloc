/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/03 22:33:03 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

void *find_in_bin(t_arena *arena, size_t size) {

	if (size <= g_manager.options.MXFAST) {
		int index = (size + 7) / 8 - 1;
		if (arena->fastbin[index]) {
			t_chunk *chunk = (t_chunk *)arena->fastbin[index];
			arena->fastbin[index] = ; // puntero a fd
			t_chunk *next = GET_FD(chunk); // deberia ser el siguiente chunk
			next->prev_size = size;
			next->size |= PREV_INUSE;
			return (GET_PTR(chunk));
		}
	}
	(void) arena;
	(void) size;
	return (NULL);
}

// cantidad_fastbins = MXFAST / 8
// tamaño_del_chunk = (índice + 1) * 8
// tamaño_máximo_real = cantidad_fastbins * 8