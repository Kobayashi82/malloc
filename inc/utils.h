/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 09:14:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/30 13:43:26 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "unistd.h"

#pragma endregion

#pragma region "Methods"

	// STRING
	size_t	ft_strlen(const char *str);
	int		ft_strcmp(const char *s1, const char *s2);
	int		ft_strlcpy(char *dst, const char *src, int dstsize);
	int		ft_strlcat(char *dst, const char *src, int dstsize);
	char	*ft_strchr(const char *str, int c);
	
	// NUMBER
	int		ft_atoi(const char *str);
	void	ft_itoa_buffered(int n, char *buffer, size_t size);
	int		ft_isdigit_s(char *str);
	int		ft_max(int n1, int n2);
	int		ft_min(int n1, int n2);

	// ATOMIC PRINTF
	int		aprintf(int fd, char const *format, ...);

#pragma endregion
