/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   number.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:01:29 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/28 13:14:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "utils.h"

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

#pragma region "ITOA (BUFFERED)"

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

#pragma endregion

#pragma region "IS DIGIT"

	int	ft_isdigit_s(char *str) {
		if (str && (*str == '+' || *str == '-')) str++;
		if (!str || *str == '\0') return (0);
		while (*str) {
			if (*str < '0' || *str > '9') return (0);
			str++;
		}

		return (1);
	}

#pragma endregion

#pragma region "MAX"

	int	ft_max(int n1, int n2) { if (n1 > n2) { return (n1); } return (n2); }

#pragma endregion

#pragma region "MIN"

	int ft_min(int n1, int n2) { if (n1 < n2) { return (n1); } return (n2); }

#pragma endregion

#pragma region "Is Power of Two"

	int is_power_of_two(size_t n) { return (n != 0 && (n & (n - 1)) == 0); }

#pragma endregion
