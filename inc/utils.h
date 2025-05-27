/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 09:14:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/27 11:35:43 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Methods"

	int		ft_strcmp(const char *s1, const char *s2);
	int		ft_strncmp(const char *s1, const char *s2, int n);
	int		ft_isdigit_s(char *str);
	int		ft_atoi(const char *str);
	void	*ft_memset(void *b, int c, int len);
	int		ft_strlcpy(char *dst, const char *src, int dstsize);
	char	*ft_strchr(const char *str, int c);
	size_t	ft_strlen(const char *str);
	void	*ft_memcpy(void *dst, const void *src, int n);
	void	ft_itoa_buffered(int n, char *buffer, size_t size);
	int		ft_strlcat(char *dst, const char *src, int dstsize);
	int		ft_printf(int fd, char const *format, ...);

#pragma endregion
