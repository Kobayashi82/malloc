/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:29:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/28 18:56:34 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#define FREELIST_SIZE	32	// MAX = INVALID_INDEX

#pragma region "Variables"

	#pragma region "Enumerators"

		typedef enum se_error {
			MTX_INIT = 20,
			MTX_LOCK = 21,
			MTX_UNLOCK = 22,
			MTX_DESTROY = 23,
			NO_MEMORY = 30
		} e_error;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	unsigned char	get_zonetype(size_t size);
	unsigned char	get_freelist_index(size_t size);
	void			print_freelist_ranges();

	// Main functions
	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);

#pragma endregion
