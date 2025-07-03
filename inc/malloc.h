/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:20:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 14:58:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

	#define M_ARENA_MAX			-8		// Maximum number of arenas allowed
	#define M_ARENA_TEST		-7		// Number of arenas at which a hard limit on arenas is computed
	#define M_PERTURB			-6		// Sets memory to the PERTURB value on allocation, and to value ^ 255 on free
	#define M_CHECK_ACTION		-5		// Behaviour on abort errors (0: abort, 1: warning, 2: silence)
	#define M_MXFAST			 1		// Max size (bytes) for fastbin allocations
	#define M_MIN_USAGE			 3		// Heaps under this usage % are skipped (unless all are under)
	#define M_DEBUG				 7		// Enables debug mode (1: error, 2: system)
	#define M_LOGGING			 8		// Enables logging mode (1: to file, 2: to stderr)

#pragma region "Methods"

	// Main
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);
	void	*calloc(size_t nmemb, size_t size);

	// Extra
	void	*reallocarray(void *ptr, size_t nmemb, size_t size);
	void	*aligned_alloc(size_t alignment, size_t size);
	void	*memalign(size_t alignment, size_t size);
	int		posix_memalign(void **memptr, size_t alignment, size_t size);
	size_t	malloc_usable_size(void *ptr);
	void	*valloc(size_t size);
	void	*pvalloc(size_t size);

	// Debug
	int		mallopt(int param, int value);
	void	show_alloc_mem();
	void	show_alloc_mem_ex(void *ptr, size_t offset, size_t length);
	void	show_alloc_history();

#pragma endregion
