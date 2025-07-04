/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   allocation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:56:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/04 14:23:50 by vzurera-         ###   ########.fr       */
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
			if (print_log(1))					aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
			errno = EINVAL; return (NULL);
		}

		if (!size) return (allocate_zero(source));
		if (!arena_find()) {
			if (print_log(1))					aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
			errno = ENOMEM; return (NULL);
		}

		void *ptr = NULL;

		mutex(&tcache->mutex, MTX_LOCK);

			bool is_large = ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK;

			if (is_large) {
				ptr = heap_create(tcache, LARGE, size, alignment);
			} else {
				size_t user_chunk_size = ALIGN(size + sizeof(t_chunk));
				size_t worst_case_total = (alignment - 1 + sizeof(t_chunk) + sizeof(void *) + sizeof(uint32_t)) + user_chunk_size;

				int type = (worst_case_total > TINY_CHUNK) ? SMALL : TINY;
				t_heap *heap = get_bestheap(tcache, type, worst_case_total);
				if (!heap) {
					if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
					mutex(&tcache->mutex, MTX_UNLOCK);
					return (NULL);
				}

				uintptr_t	aligned_user_addr = ALIGN_UP((uintptr_t)(char *)GET_PTR(heap->top_chunk), alignment);
				size_t		padding_needed = ((char *)aligned_user_addr - sizeof(t_chunk)) - (char *)heap->top_chunk;

				if (!padding_needed) {
					if (GET_SIZE(heap->top_chunk) < user_chunk_size) {
						heap = heap_create(tcache, heap->type, (heap->type == TINY) ? TINY_SIZE : SMALL_SIZE, ALIGNMENT);
						if (!heap) {
							if (print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
							mutex(&tcache->mutex, MTX_UNLOCK);
							return (NULL);
						}
					}
					t_chunk *chunk = split_top_chunk(heap, user_chunk_size);
					if (!chunk) {
						if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
						mutex(&tcache->mutex, MTX_UNLOCK);
						return (NULL);
					}
					heap->free -= user_chunk_size;
					ptr = GET_PTR(chunk);
				} else {
					size_t min_padding_size = sizeof(t_chunk) + sizeof(void *) + sizeof(uint32_t);
					if (padding_needed < min_padding_size) {
						aligned_user_addr += ALIGN_UP(min_padding_size - padding_needed, alignment);
						padding_needed = ((char *)aligned_user_addr - sizeof(t_chunk)) - (char *)heap->top_chunk;
					}

					size_t total_needed = padding_needed + user_chunk_size;
					if (GET_SIZE(heap->top_chunk) < total_needed) {
						heap = heap_create(tcache, heap->type, (heap->type == TINY) ? TINY_SIZE : SMALL_SIZE, ALIGNMENT);
						if (!heap) {
							if (print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
							mutex(&tcache->mutex, MTX_UNLOCK);
							return (NULL);
						}
					}
					t_chunk *padding_chunk = split_top_chunk(heap, total_needed);
					if (!padding_chunk) {
						if (print_log(1)) 		aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
						mutex(&tcache->mutex, MTX_UNLOCK);
						return (NULL);
					}

					size_t original_flags = padding_chunk->size & (HEAP_TYPE | PREV_INUSE | MMAP_CHUNK);
					padding_chunk->size = (padding_needed - sizeof(t_chunk)) | original_flags;
					SET_POISON(GET_PTR(padding_chunk));

					t_chunk *user_chunk = (t_chunk *)((char *)aligned_user_addr - sizeof(t_chunk));
					user_chunk->size = (user_chunk_size - sizeof(t_chunk)) | PREV_INUSE | (original_flags & HEAP_TYPE);
					SET_PREV_SIZE(user_chunk, padding_needed - sizeof(t_chunk));
					SET_PREV_SIZE(heap->top_chunk, (user_chunk_size - sizeof(t_chunk)));

					if (padding_needed <= (size_t)g_manager.options.MXFAST)		link_chunk(padding_chunk, padding_needed, FASTBIN, tcache, heap);
					else														link_chunk(padding_chunk, padding_needed, UNSORTEDBIN, tcache, heap);

					heap->free_chunks++;
					heap->free += padding_needed;
					heap->free -= total_needed;
					ptr = GET_PTR(user_chunk);
				}
			}

			if (ptr && g_manager.options.PERTURB) ft_memset(ptr, g_manager.options.PERTURB ^ 0xFF, GET_SIZE((t_chunk *)GET_HEAD(ptr)));

			if (ptr && print_log(0))			aprintf(g_manager.options.fd_out, 1, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			if (!ptr && print_log(1))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

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

		if (!arena_find()) {
			if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated 0 bytes\n");
			errno = ENOMEM; return (NULL);
		}

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

	void *allocate(char *source, size_t size) {
		if (!source || !*source) source = "UNKOWN";

		if (size > SIZE_MAX - sizeof(t_chunk)) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
			errno = ENOMEM; return (NULL);
		}

		if (!size) return (allocate_zero(source));
		if (!arena_find()) {
			if (print_log(1))			aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
			errno = ENOMEM; return (NULL);
		}

		void *ptr = NULL;

		mutex(&tcache->mutex, MTX_LOCK);

			bool is_large = ALIGN(size + sizeof(t_chunk)) > SMALL_CHUNK;

			ptr = find_memory(tcache, size);

			if (ptr && (g_manager.options.PERTURB || (!is_large && !ft_strcmp(source, "CALLOC")))) {
				if (!is_large && !ft_strcmp(source, "CALLOC")) ft_memset(ptr, 0, GET_SIZE((t_chunk *)GET_HEAD(ptr)));
				else if (ft_strcmp(source, "CALLOC")) ft_memset(ptr, g_manager.options.PERTURB ^ 0xFF, GET_SIZE((t_chunk *)GET_HEAD(ptr)));
			}

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
