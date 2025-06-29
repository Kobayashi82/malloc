/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:20:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 13:04:56 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

	#define M_ARENA_MAX					-8								//
	#define M_ARENA_TEST				-7								//
	#define M_PERTURB					-6								//
	#define M_CHECK_ACTION				-5								//
	#define M_MXFAST			 		 1								//
	#define M_MIN_USAGE_PERCENT			 3								// Si una zona esta menos usada que esto, no usarla (pero si todas estan por debajo del threshold, usar la de mayor tamaño)
	#define M_DEBUG						 7								// (DEBUG) Enable debug mode
	#define M_LOGGING					 8								// (DEBUG) Captura backtrace con backtrace() y lo guardas junto con cada allocación.
	#define M_LOGFILE					 9								// (DEBUG) Con diferentes comportamientos según el valor:

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

	// Debug
	int		mallopt(int param, int value);
	void	show_alloc_history();
	void	show_alloc_mem();
	void	show_alloc_mem_ex(void *ptr, size_t offset, size_t length);

#pragma endregion
