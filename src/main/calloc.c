/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 12:25:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/25 13:19:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Calloc"

	__attribute__((visibility("default")))
	void *calloc(size_t nmemb, size_t size) {
		ensure_init();

		void	*ptr = NULL;
		size_t	total = 0;

		if (!nmemb || !size)			return (malloc(0));
		if (nmemb > SIZE_MAX / size)	return (NULL);

		total = nmemb * size;
		if (g_manager.options.DEBUG) aprintf(2, "\t\t [CALLOC] Asking for %d bytes\n", size);
		if ((ptr = malloc(total))) ft_memset(ptr, 0, total);

		return (ptr);
	}

#pragma endregion
