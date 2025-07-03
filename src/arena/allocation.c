/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   allocation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:56:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 15:01:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Check Digit"

	int check_digit(void *ptr1, void *ptr2) {
		if (!ptr1 || !ptr2) return (0);

		uintptr_t val1 = (uintptr_t)ptr1;
		uintptr_t val2 = (uintptr_t)ptr2;

		while (val1 >= 0x10) val1 /= 0x10;
		while (val2 >= 0x10) val2 /= 0x10;

		return ((val1 & 0xF) == (val2 & 0xF));
	}

#pragma endregion

#pragma region "Allocate Aligned"

	void *allocate_aligned(char *source, size_t alignment, size_t size) {
		if (!source || !*source) source = "UNKOWN";

		if (size > SIZE_MAX - sizeof(t_chunk)) { errno = ENOMEM; return (NULL); }

		if (alignment < sizeof(void *) || !is_power_of_two(alignment)) {
			if (print_log(1)) aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
			errno = EINVAL;
			return (NULL);
		}

		if (!size)			return (allocate_zero(source));
		if (!arena_find())	return (NULL);

		void *ptr = NULL;

		mutex(&tcache->mutex, MTX_LOCK);

			bool is_large = ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK;
			unsigned char perturb = 0;

			if (is_large) {
				ptr = heap_create(tcache, LARGE, size, alignment);
			} else {
				// Para chunks pequeños y medianos, usar la lógica simplificada
				// Calcular el tamaño del chunk del usuario
				size_t user_chunk_size = ALIGN(size + sizeof(t_chunk));
				
				// Calcular el peor caso de padding necesario para la alineación
				size_t max_padding_needed = alignment - 1 + sizeof(t_chunk) + (2 * sizeof(void *)); // peor caso + header + datos mínimos
				
				// Tamaño total en el peor caso
				size_t worst_case_total = max_padding_needed + user_chunk_size;
				
				// Determinar tipo de heap basado en el peor caso
				int type = (worst_case_total > TINY_CHUNK) ? SMALL : TINY;
				t_heap *heap = get_bestheap(tcache, type, worst_case_total);
				if (!heap) {
					mutex(&tcache->mutex, MTX_UNLOCK);
					return (NULL);
				}

				// Calcular donde está actualmente el top chunk
				char *current_start = (char *)heap->top_chunk;
				char *current_user_ptr = (char *)GET_PTR(heap->top_chunk);
				
				// Calcular donde debería estar el user pointer alineado
				uintptr_t aligned_user_addr = ALIGN_UP((uintptr_t)current_user_ptr, alignment);
				char *aligned_user_ptr = (char *)aligned_user_addr;
				char *aligned_chunk_start = aligned_user_ptr - sizeof(t_chunk);
				
				// Calcular cuánto padding necesitamos
				size_t padding_needed = aligned_chunk_start - current_start;
				
				// Si no necesitamos padding, usar directamente
				if (padding_needed == 0) {
					t_chunk *chunk = split_top_chunk(heap, user_chunk_size);
					if (!chunk) {
						mutex(&tcache->mutex, MTX_UNLOCK);
						return (NULL);
					}
					heap->free -= user_chunk_size;
					ptr = GET_PTR(chunk);
				} else {
					// Si necesitamos padding, verificar que sea suficiente para un chunk válido
					size_t min_padding_size = sizeof(t_chunk) + (2 * sizeof(void *)); // header + 2 punteros
					
					if (padding_needed < min_padding_size) {
						// Padding demasiado pequeño, avanzar al siguiente múltiplo de alignment
						size_t adjustment = min_padding_size - padding_needed;
						adjustment = ALIGN_UP(adjustment, alignment);
						aligned_user_addr += adjustment;
						aligned_user_ptr = (char *)aligned_user_addr;
						aligned_chunk_start = aligned_user_ptr - sizeof(t_chunk);
						padding_needed = aligned_chunk_start - current_start;
					}
					
					// Ahora dividir el espacio total necesario
					size_t total_needed = padding_needed + user_chunk_size;
					t_chunk *big_chunk = split_top_chunk(heap, total_needed);
					if (!big_chunk) {
						mutex(&tcache->mutex, MTX_UNLOCK);
						return (NULL);
					}
					
					// Crear chunk de padding
					t_chunk *padding_chunk = big_chunk;
					size_t padding_data_size = padding_needed - sizeof(t_chunk);
					size_t original_flags = padding_chunk->size & (HEAP_TYPE | PREV_INUSE | MMAP_CHUNK);
					padding_chunk->size = padding_data_size | original_flags;
					SET_POISON(GET_PTR(padding_chunk));
					
					// Crear chunk del usuario
					t_chunk *user_chunk = (t_chunk *)aligned_chunk_start;
					size_t user_data_size = user_chunk_size - sizeof(t_chunk);
					user_chunk->size = user_data_size | PREV_INUSE | (original_flags & HEAP_TYPE);
					user_chunk->magic = 0; // Se configurará después
					
					// Configurar prev_size para el siguiente chunk
					SET_PREV_SIZE(heap->top_chunk, user_data_size);
					
					// Añadir el padding chunk a los bins
					size_t padding_total_size = padding_needed;
					if (padding_total_size <= (size_t)g_manager.options.MXFAST) {
						link_chunk(padding_chunk, padding_total_size, FASTBIN, tcache, heap);
					} else {
						link_chunk(padding_chunk, padding_total_size, UNSORTEDBIN, tcache, heap);
					}
					heap->free_chunks++;
					heap->free += padding_total_size;
					heap->free -= total_needed; // Restar el total usado
					
					ptr = GET_PTR(user_chunk);
				}
			}

			if (g_manager.options.PERTURB && !is_large) perturb = g_manager.options.PERTURB;
			if (ptr && perturb) ft_memset(ptr, perturb, GET_SIZE((t_chunk *)GET_HEAD(ptr)));

			if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			if (!ptr && print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

			if (ptr) {
				SET_MAGIC(ptr);
				tcache->alloc_count++;
			}

		mutex(&tcache->mutex, MTX_UNLOCK);

		if (!ptr) errno = ENOMEM;
		return (ptr);
	}

#pragma endregion

#pragma region "Allocate Zero"

	void *allocate_zero(char *source) {
		if (!source || !*source) source = "UNKOWN";

		if (!arena_find()) return (NULL);

		void *ptr = NULL;

		mutex(&g_manager.mutex, MTX_LOCK);

			size_t aligned_offset = (g_manager.alloc_zero_counter * ALIGNMENT);
			g_manager.alloc_zero_counter++;

		mutex(&g_manager.mutex, MTX_UNLOCK);

		ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
		if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated 0 bytes\n", ptr, source);
		if (!ptr && print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated 0 bytes\n");

		if (ptr) {
			mutex(&tcache->mutex, MTX_LOCK);

				tcache->alloc_count++;

			mutex(&tcache->mutex, MTX_UNLOCK);
		}
		if (!ptr) errno = ENOMEM;
		return (ptr);
	}

#pragma endregion

#pragma region "Allocate"

	void *allocate(char *source, size_t size, unsigned char perturb) {
		if (!source || !*source) source = "UNKOWN";

		if (size > SIZE_MAX - sizeof(t_chunk)) { errno = ENOMEM; return (NULL); }
		if (!size)			return (allocate_zero(source));
		if (!arena_find())	return (NULL);

		void *ptr = NULL;

		mutex(&tcache->mutex, MTX_LOCK);

			bool is_large = ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK;

			ptr = find_memory(tcache, size);

			if (g_manager.options.PERTURB && !is_large && ft_strcmp(source, "CALLOC")) perturb = g_manager.options.PERTURB;
			if (ptr && (perturb || (!perturb && !is_large && !ft_strcmp(source, "CALLOC"))))
				ft_memset(ptr, perturb, GET_SIZE((t_chunk *)GET_HEAD(ptr)));

			if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			if (!ptr && print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated 2 %u bytes\n", size);

			if (ptr) {
				SET_MAGIC(ptr);
				tcache->alloc_count++;
			}

		mutex(&tcache->mutex, MTX_UNLOCK);

		if (!ptr) errno = ENOMEM;
		return (ptr);
	}

#pragma endregion
