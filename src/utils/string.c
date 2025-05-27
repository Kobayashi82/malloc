/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 21:58:15 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/27 22:11:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "STRLEN"

	size_t ft_strlen(const char *str) {
		size_t i = 0;

		while (str && str[i]) ++i;
		return (i);
	}

#pragma endregion

#pragma region "STRCMP"

	int	ft_strcmp(const char *s1, const char *s2) {
		if (!s1 || !s2) return (-1);
		while (*s1 && (*s1 == *s2)) { s1++; s2++; }
		return (*(unsigned char *)s1 - *(unsigned char *)s2);
	}

#pragma endregion

#pragma region "STRLCPY"

	int ft_strlcpy(char *dst, const char *src, int dstsize) {
		int		i = 0, srclen = 0;

		if (!src) return (0);
		while (src[srclen]) ++srclen;
		if (dstsize > 0) {
			while (dstsize > 0 && src[i] && i < dstsize - 1) { dst[i] = src[i]; ++i; }
			dst[i] = '\0';
		}

		return (srclen);
	}

#pragma endregion

#pragma region "STRLCAT"

	int ft_strlcat(char *dst, const char *src, int dstsize) {
		int src_len = 0, dest_len = 0, i;

		while (dst[dest_len] && dest_len < dstsize) dest_len++;
		while (src[src_len]) src_len++;
		if (dstsize == 0 || dest_len >= dstsize) return (dest_len + src_len);
		i = dest_len - 1;
		while (++i < dstsize - 1 && src[i - dest_len]) dst[i] = src[i - dest_len];
		dst[i] = '\0';
		return (dest_len + src_len);
	}

#pragma endregion

#pragma region "STRCHR"

	char *ft_strchr(const char *str, int c) {
		if (!str) return (NULL);
		while (*str) { if (*str == (char)c) { break; } str++; }
		if (*str == (char)c) return ((char *)str);

		return (NULL);
	}

#pragma endregion
