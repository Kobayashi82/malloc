/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   freelist.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 10:56:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/25 13:52:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"

// cambiar freelist a fastbin
// limitar tamaños de fastbin a 16 - 128 o hasta 160 en sistemas de 64 bits
// Ejemplo: En 64 bits, los tamaños suelen ser 16, 24, 32, ..., 160 bytes (múltiplos de 16 + 8 debido al alineamiento).
// LIFO (first in - first out)
// ... investigar mas sobre fastbin, smallbin y demas.
// no es tan sencillo como un freelist y ademas esta el tcache que no he investigado aun

unsigned char get_zonetype(size_t size) {
	if		(size <= TINY_MAX)		return (TINY);
	else if (size <= SMALL_MAX)		return (SMALL);

	return (INVALID_INDEX);
}

unsigned char get_freelist_index(size_t size) {
	int max, base;

	if		(size == 0)				size = 1;
	if 		(size <= TINY_MAX)		{ max = TINY_MAX;	base = 1; }
	else if (size <= SMALL_MAX)		{ max = SMALL_MAX;	base = TINY_MAX + 1; }
	else							return INVALID_INDEX;

	int range = max - base + 1;
	int step = (range + FREELIST_SIZE - 1) / FREELIST_SIZE;
	unsigned char index = (size - base) / step;

	return (index >= FREELIST_SIZE) ? FREELIST_SIZE - 1 : index;
}

void print_freelist_ranges() {
	int tiny_base = 1;
	int tiny_range = TINY_MAX - tiny_base + 1;
	int tiny_step = (tiny_range + FREELIST_SIZE - 1) / FREELIST_SIZE;

	printf("TINY (%d bytes):\n", TINY_MAX);
	for (int i = 0; i < FREELIST_SIZE; ++i) {
		int min = tiny_base + i * tiny_step;
		int max = tiny_base + (i + 1) * tiny_step - 1;
		if (max > TINY_MAX) max = TINY_MAX;
		printf("  [%2d] %4d - %4d bytes\n", i, min, max);
	}

	int small_base = TINY_MAX + 1;
	int small_range = SMALL_MAX - small_base + 1;
	int small_step = (small_range + FREELIST_SIZE - 1) / FREELIST_SIZE;

	printf("\nSMALL (%d bytes):\n", SMALL_MAX);
	for (int i = 0; i < FREELIST_SIZE; ++i) {
		int min = small_base + i * small_step;
		int max = small_base + (i + 1) * small_step - 1;
		if (max > SMALL_MAX) max = SMALL_MAX;
		printf("  [%2d] %5d - %5d bytes\n", i, min, max);
	}
}
