/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 21:42:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 18:05:01 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

	#include <string.h>

#pragma endregion

#pragma region "Main"

	int main() {
		mallopt(M_DEBUG, 1);

		char *popo1 = malloc(100);
		char *popo2 = malloc(100);
		strcpy(popo1, "Hello World!");
		malloc_stats();
		show_alloc_mem_ex(popo1, 0, 0);
		free(popo1);
		show_alloc_history();
		// popo += 8;

	}

#pragma endregion

#pragma region "Information"

	// gcc -g -Wno-free-nonheap-object -o testing testing.c -I./inc -L./build/lib -lft_malloc -Wl,-rpath=./build/lib -pthread
	//
	// -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
	// -lft_malloc				= -l busca lib + ft_malloc + .so
	// -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime
