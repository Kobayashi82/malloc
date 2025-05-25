/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 18:26:45 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/25 19:06:44 by vzurera-         ###   ########.fr       */
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
