/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   allocation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:56:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 13:33:25 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Allocate Aligned"

	#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))

	void *allocate_aligned(char *source, size_t alignment, size_t size) {
		if (!source || !*source) source = "UNKOWN";

		if (alignment < sizeof(void *) || !is_power_of_two(alignment)) {
			if (print_log(0))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
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
				// 1. Calcular el peor caso de padding para ambos tipos
				size_t max_padding_data_tiny = alignment - 1;
				size_t max_padding_data_small = alignment - 1;

				// 2. Tamaños mínimos según el tipo de heap
				size_t min_chunk_data_tiny = 16;
				size_t min_chunk_data_small = TINY_CHUNK + 16;

				// 3. Ajustar padding si es muy pequeño para cada tipo
				size_t max_padding_total_tiny = max_padding_data_tiny + sizeof(t_chunk);
				if (max_padding_data_tiny < min_chunk_data_tiny) {
					max_padding_total_tiny = min_chunk_data_tiny + sizeof(t_chunk);
				}

				size_t max_padding_total_small = max_padding_data_small + sizeof(t_chunk);
				if (max_padding_data_small < min_chunk_data_small) {
					max_padding_total_small = min_chunk_data_small + sizeof(t_chunk);
				}

				// 4. Calcular tamaño total para cada tipo
				size_t new_chunk_total = ALIGN(size + sizeof(t_chunk));
				size_t total_if_tiny = max_padding_total_tiny + new_chunk_total;
				size_t total_if_small = max_padding_total_small + new_chunk_total;

				// 5. Determinar tipo: si cabe en TINY, usar TINY; si no, usar SMALL
				int type;
				size_t total_worst_case;
				size_t min_chunk_data;

				if (total_if_tiny <= TINY_CHUNK) {
					type = TINY;
					total_worst_case = total_if_tiny;
					min_chunk_data = min_chunk_data_tiny;
				} else {
					type = SMALL;
					total_worst_case = total_if_small;
					min_chunk_data = min_chunk_data_small;
				}

				// 6. Obtener heap del tipo correcto
				t_heap *heap = get_bestheap(tcache, type, total_worst_case);
				if (!heap) {
					mutex(&tcache->mutex, MTX_UNLOCK);
					return (NULL);
				}

				// 7. Calcular padding real
				char *current_ptr = (char *)GET_PTR(heap->top_chunk);
				char *aligned_user_ptr = (char *)ALIGN_UP((uintptr_t)(current_ptr + sizeof(t_chunk)), alignment);
				char *new_chunk_start = aligned_user_ptr - sizeof(t_chunk);
				size_t real_padding_total = new_chunk_start - current_ptr;
				size_t real_padding_data = real_padding_total - sizeof(t_chunk);

				// 8. Ajustar si el padding es muy pequeño
				if (real_padding_total > 0 && real_padding_data < min_chunk_data) {
					aligned_user_ptr += alignment;
					new_chunk_start = aligned_user_ptr - sizeof(t_chunk);
					real_padding_total = new_chunk_start - current_ptr;
					real_padding_data = real_padding_total - sizeof(t_chunk);
				}

				// 9. Calcular el tamaño total real necesario
				size_t total_real = real_padding_total + ALIGN(size + sizeof(t_chunk));

				// 10. Dividir el top chunk del tamaño total
				t_chunk *big_chunk = split_top_chunk(heap, total_real);
				if (!big_chunk) {
					mutex(&tcache->mutex, MTX_UNLOCK);
					return (NULL);
				}

				// 11. Si hay padding, crear el chunk de padding
				t_chunk *user_chunk;
				if (real_padding_total > 0) {
					// Configurar el chunk de padding - preservar flags originales
					t_chunk *padding_chunk = big_chunk;
					size_t original_flags = padding_chunk->size & (HEAP_TYPE | PREV_INUSE | MMAP_CHUNK | TOP_CHUNK);
					padding_chunk->size = real_padding_data | original_flags;
					SET_POISON(GET_PTR(padding_chunk));

					// El chunk del usuario empieza después del padding
					user_chunk = (t_chunk *)((char *)big_chunk + real_padding_total);
					// El chunk del usuario tiene PREV_INUSE (porque el anterior ya no está libre) 
					// y el mismo HEAP_TYPE que el chunk de padding
					size_t heap_type_flag = original_flags & HEAP_TYPE;

					size_t user_chunk_size = ALIGN(size + sizeof(t_chunk)) - sizeof(t_chunk);
					user_chunk->size = user_chunk_size | PREV_INUSE | heap_type_flag;

					SET_PREV_SIZE(user_chunk, GET_SIZE(padding_chunk));
					SET_PREV_SIZE(heap->top_chunk, GET_SIZE(user_chunk));
					link_chunk(padding_chunk, GET_SIZE(padding_chunk), heap->type, tcache, heap);
					heap->free_chunks++;
					heap->free += GET_SIZE(padding_chunk) + sizeof(t_chunk);
				} else {
					// No hay padding, el chunk dividido es directamente el del usuario
					user_chunk = big_chunk;
					aprintf(2, 0, "PTR: %p\n", GET_PTR(user_chunk));
				}

				// 12. Devolver el puntero alineado
				ptr = GET_PTR(user_chunk);
			}

			if (g_manager.options.PERTURB && !is_large) perturb = g_manager.options.PERTURB;
			if (ptr && perturb) ft_memset(ptr, perturb, GET_SIZE((t_chunk *)GET_HEAD(ptr)));

			if		(ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			else if (print_log(0))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

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
		if		(ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated 0 bytes\n", ptr, source);
		else if (print_log(0))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated 0 bytes\n");

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

		if (!size)			return (allocate_zero(source));
		if (!arena_find())	return (NULL);

		void *ptr = NULL;

		mutex(&tcache->mutex, MTX_LOCK);

			bool is_large = ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK;

			ptr = find_memory(tcache, size);

			if (g_manager.options.PERTURB && !is_large && ft_strcmp(source, "CALLOC")) perturb = g_manager.options.PERTURB;
			if (ptr && (perturb || (!perturb && !is_large && !ft_strcmp(source, "CALLOC"))))
				ft_memset(ptr, perturb, GET_SIZE((t_chunk *)GET_HEAD(ptr)));

			if		(ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			else if (print_log(0))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

			if (ptr) {
				SET_MAGIC(ptr);
				tcache->alloc_count++;
			}

		mutex(&tcache->mutex, MTX_UNLOCK);

		if (!ptr) errno = ENOMEM;
		return (ptr);
	}

#pragma endregion
