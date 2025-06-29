/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 21:42:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 12:49:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

#pragma region "Main"

	int main() {
		mallopt(M_DEBUG, 1);

		char *popo = malloc(0);
		char *popo1 = malloc(0);
		char *popo2 = malloc(0);
		char *popo3 = malloc(0);
		char *popo4 = malloc(0);
		// popo += 8;
		free(popo);
		free(popo1);
		free(popo2);
		free(popo3);
		free(popo4);
	}

#pragma endregion

#pragma region "Information"

	// gcc -g -Wno-free-nonheap-object -o testing testing.c -I./inc -L./build/lib -lft_malloc -Wl,-rpath=./build/lib -pthread
	//
	// -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
	// -lft_malloc				= -l busca lib + ft_malloc + .so
	// -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

#pragma endregion
