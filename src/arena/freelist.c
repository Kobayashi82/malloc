/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   freelist.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 10:56:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/28 22:43:58 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "arena.h"

//	Arena
//	├── Fastbin  [0-9]  (16-160 bytes)										Arrays de listas simples (LIFO)
//	├── Smallbin [0-30] (176-512 bytes para TINY, 513-4096 para SMALL)		Doblemente enlazadas. Tamaños fijos (FIFO)
//	├── Largebin ???														Doblemente enlazada.  Ordenados internamente
//	├── Unsorted Bin														Doblemente enlazada
//	└── Zonas
//		├── Zonas TINY  (fastbin + smallbin pequeños)
//		└── Zonas SMALL (smallbin grandes)

//	Flujo de asignación malloc():
//
//		Fastbin  (si el tamaño solicitado coincide)
//		Smallbin (si el tamaño solicitado coincide exactamente)
//		Large bins
//		Unsorted bin

//	Flujo de liberación free():
//
//		Fastbins  (si el tamaño solicitado coincide)
//		Smallbins (si el tamaño solicitado coincide exactamente)
//		Large bins
//		Unsorted bin

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
