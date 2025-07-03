/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 21:42:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 21:21:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

	#include <string.h>

#pragma endregion

#pragma region "Main"

	int main() {
		mallopt(M_DEBUG, 1);
		mallopt(M_LOGGING, 2);

		char *ptr = malloc(30);
		strcpy(ptr, "Hello World!\n");
		show_alloc_mem();
		show_alloc_mem_ex(ptr, 0, 0);
		free(ptr);
		show_alloc_history();
	}

#pragma endregion

#pragma region "Information"
