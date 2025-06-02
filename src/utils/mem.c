/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mem.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 14:11:34 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/02 15:09:14 by vzurera-         ###   ########.fr       */
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

#pragma region "MEMCPY"

	void *ft_memcpy(void *dst, const void *src, int n) {
		unsigned char		*d = (unsigned char *)dst;
		const unsigned char	*s = (const unsigned char *)src;

		if (!n || (!d && !s && n)) return (dst);
		while (n-- > 0) *d++ = *s++;

		return (dst);
	}

#pragma endregion
