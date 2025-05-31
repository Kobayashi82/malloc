/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:29:51 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/31 17:45:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Methods"

	unsigned char	get_zonetype(size_t size);
	unsigned char	get_freelist_index(size_t size);
	void			print_freelist_ranges();

	void	realfree(void *ptr);

	void	free(void *ptr);
	void	*malloc(size_t size);
	void	*realloc(void *ptr, size_t size);

#pragma endregion
