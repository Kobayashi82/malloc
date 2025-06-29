/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:20:00 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 12:36:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <stddef.h>

#pragma endregion

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
	void	show_alloc_mem();
	void	show_alloc_mem_ex();
	void	show_alloc_history();

#pragma endregion
