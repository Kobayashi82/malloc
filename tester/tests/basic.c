/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 21:42:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/04 20:47:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

	#include <string.h>
	#include <stdio.h>

#pragma endregion

#pragma region "Main"

	int main() {
		mallopt(M_DEBUG, 2);
		mallopt(M_LOGGING, 0);

		char *ptr = malloc(30);
		strcpy(ptr, "Hello World!\n");
		show_alloc_mem();
		show_alloc_mem_ex(ptr, 0, 0);
		free(ptr);
		show_alloc_history();
	}

#pragma endregion
