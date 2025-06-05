/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coalescing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 11:00:49 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/05 11:54:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

// MINSIZE = 32 bits = 16 y 64 bits = 24

// Free
//
// Si fue mmap'd → munmap
// Fastbin (pequeños, sin coalescing)
// Otros chunks → coalescing (left and right) + unsortedbin
//
// Free Heap
//
// contador de frees en heap (cuando vacio)
// En free a nivel de arena subir contador en heaps (cuando vacio)
// resetear contador si malloc en el heap
//
// Malloc
//
// Si large → mmap?
// Fastbin exact match
// Smallbin exact match
// Si large → consolidar fastbins a unsortedbin
// Procesar unsortedbin → small/large bins (coalescing)
// Si large → buscar en largebin y superiores (split si > MINSIZE)
// Si quedan fastbins → consolidar y repetir
// Split top chunk
//