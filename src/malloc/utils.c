/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 18:26:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/26 21:14:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "STR_CMP"

	#pragma region "StrCmp"

		int	ft_strcmp(const char *s1, const char *s2) {
			if (!s1 || !s2) return (-1);
			while (*s1 && (*s1 == *s2)) { s1++; s2++; }
			return (*(unsigned char *)s1 - *(unsigned char *)s2);
		}

	#pragma endregion

	#pragma region "StrNCmp"

		int	ft_strncmp(const char *s1, const char *s2, int n) {
			unsigned char	*str1, *str2;
			int				i = 0;

			if (!s1 || !s2) return (-1);
			str1 = (unsigned char *)s1;
			str2 = (unsigned char *)s2;
			while (*str1 && (*str1 == *str2) && i < n && n > 0) { ++str1; ++str2; ++i; }
			if ((!*str1 && !*str2) || i == n) return (0);
			return (*str1 - *str2);
		}

	#pragma endregion

#pragma endregion

#pragma region "ATOI"

	int ft_atoi(const char *str) {
		int sign = 1, num = 0;

		if (!str) return (0);
		while ((*str >= 9 && *str <= 13) || *str == 32) ++str;
		if ((*str == '+' || *str == '-') && *str++ == '-') sign = -sign;
		while (*str >= '0' && *str <= '9') num = (num * 10) + (*str++ - '0');
		return (num * sign);
	}

#pragma endregion

#pragma region "Is_Digit"

	int	ft_isdigit(int c) { return (c >= '0' && c <= '9'); }

#pragma endregion

#pragma region "Is_Digit STR"

	int	ft_isdigit_s(char *str) {
		if (str && (*str == '+' || *str == '-')) str++;
		if (!str || *str == '\0') return (0);
		while (*str) {
			if (!ft_isdigit(*str++)) return (0);
		}

		return (1);
	}

#pragma endregion

#pragma region "MEMSET"

	void *ft_memset(void *b, int c, int len) {
		unsigned char	*p = b;

		while (len--) *p++ = (unsigned char)c;

		return (b);
	}

#pragma endregion

#pragma region "STR_CPY"

	#pragma region "StrCpy"

		char *ft_strcpy(char *dst, const char *src) {
			int	i = -1;

			if (!dst || !src) return (NULL);
			while (src[++i]) dst[i] = src[i];
			dst[i] = '\0';
			return (dst);
		}

	#pragma endregion

	#pragma region "StrNCpy"

		char *ft_strncpy(char *dest, const char *src, int n) {
			int	i = -1;

			while (++i < n && src[i]) dest[i] = src[i];
			while (i < n) dest[i++] = '\0';

			return (dest);
		}

	#pragma endregion

	#pragma region "StrLCpy"

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

#pragma endregion

#pragma region "STR_CHR"

	#pragma region "StrChr"

		char *ft_strchr(const char *str, int c) {
			if (!str) return (NULL);
			while (*str) { if (*str == (char)c) { break; } str++; }
			if (*str == (char)c) return ((char *)str);

			return (NULL);
		}

	#pragma endregion

	#pragma region "StrRChr"

		char *ft_strrchr(const char *str, int c) {
			int len = ft_strlen(str);
			if (!str) return (NULL);
			str += len;
			while (len-- > 0) { if (*str == (char)c) { break; } str--; }
			if (*str == (char)c) return ((char *)str);

			return (NULL);
		}

	#pragma endregion

#pragma endregion

#pragma region "STR_LEN"

	#pragma region "StrLen"

		size_t ft_strlen(const char *str) {
			size_t i = 0;

			while (str && str[i]) ++i;
			return (i);
		}

	#pragma endregion

	#pragma region "StrLen (Lines)"

		size_t ft_strlenl(const char *str) {
			size_t lines = 0;

			if (str) lines++;
			while (str && *str) { if (*str++ == '\n') lines++; }
			return (lines);
		}

	#pragma endregion

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

	void ft_itoa_buffered(int n, char *buffer, size_t size) {
		if (!buffer || size == 0) return;

		if (n == 0) {
			if (size >= 2) {
				buffer[0] = '0';
				buffer[1] = '\0';
			}
			return;
		}

		int len = 0, sign = 1, tmp = n;
		if (n < 0) { sign = -1; len++; }
		
		while (tmp != 0) {
			tmp /= 10;
			len++;
		}

		if (len >= (int)size) return;
		buffer[len] = '\0';

		while (n != 0) {
			buffer[--len] = '0' + (sign * (n % 10));
			n /= 10;
		}

		if (sign == -1) buffer[0] = '-';
	}

#pragma region "STR_CAT"

	#pragma region "StrCat"

		char *ft_strcat(char *dest, const char *src) {
			int dest_len = 0, i = -1;

			while (dest[dest_len]) dest_len++;
			while (src[++i]) dest[dest_len + i] = src[i];
			dest[dest_len + i] = '\0';
			return (dest);
		}

	#pragma endregion

	#pragma region "StrNCat"

		char *ft_strncat(char *dest, const char *src, int n) {
			int dest_len = 0, i = -1;

			while (dest[dest_len]) dest_len++;
			while (++i < n && src[i]) dest[dest_len + i] = src[i];
			dest[dest_len + i] = '\0';
			return (dest);
		}

	#pragma endregion

	#pragma region "StrLCat"

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

#pragma endregion
