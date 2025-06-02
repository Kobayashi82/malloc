/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mem.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 14:11:34 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/02 13:52:09 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "utils.h"

#pragma endregion

#pragma region "MEMSET"

	void *ft_memset(void *b, int c, size_t len) {
		unsigned char	*p = b;

		while (len--) *p++ = (unsigned char)c;

		return (b);
	}

#pragma endregion
