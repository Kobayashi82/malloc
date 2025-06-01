/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mem.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 14:11:34 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 15:05:56 by vzurera-         ###   ########.fr       */
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

#pragma region "Alloc"

	void *internal_alloc(size_t size) {
		void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		return (ptr == MAP_FAILED ? NULL : ptr);
	}

#pragma endregion

#pragma region "Free"

	int internal_free(void *ptr, size_t size) {
		if (ptr && size > 0) return (munmap(ptr, size));
		return (0);
	}

#pragma endregion