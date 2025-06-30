/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   allocation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:56:07 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 11:37:30 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Allocate Aligned"

	void *allocate_aligned(char *source, size_t alignment, size_t size) {
		if (!source || !*source) source = "UNKOWN";

		if (alignment < sizeof(void *) || !is_power_of_two(alignment)) {
			if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);
			errno = EINVAL;
			return (NULL);
		}

		if (!size)			return (allocate_zero(source));
		if (!arena_find())	return (NULL);

		void *ptr = NULL;

		mutex(&tcache->mutex, MTX_LOCK);

			// Logica de allocation
		
			if		(ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			else if (g_manager.options.DEBUG)			aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

			if (ptr) SET_MAGIC(ptr);

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

			size_t aligned_offset = (g_manager.zero_malloc_counter * ALIGNMENT);
			g_manager.zero_malloc_counter++;

		mutex(&g_manager.mutex, MTX_UNLOCK);

		ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
		if		(ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [%s] Allocated 0 bytes\n", ptr, source);
		else if (g_manager.options.DEBUG)			aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated 0 bytes\n");

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

			if		(ptr && g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t [%s] Allocated %u bytes\n", ptr, source, size);
			else if (g_manager.options.DEBUG)			aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

			if (ptr) {
				SET_MAGIC(ptr);
				tcache->alloc_count++;
			}

		mutex(&tcache->mutex, MTX_UNLOCK);

		if (!ptr) errno = ENOMEM;
		return (ptr);
	}

#pragma endregion
