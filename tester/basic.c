/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 21:42:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/03 21:15:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

	#include <stdlib.h>
	#include <signal.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <pthread.h>
	#include <sys/mman.h>
	#include <errno.h>
	#include <string.h>

#pragma endregion

#pragma region "Defines"

	#define M_ARENA_MAX					-8			//
	#define M_ARENA_TEST				-7			//
	#define M_PERTURB					-6			//
	#define M_CHECK_ACTION				-5			//
	#define M_MXFAST			 		 1			//
	#define M_FRAG_PERCENT			 	 2			// Si una zona esta mas fragmentada que esto, no usarla (crear nueva si es necesario)
	#define M_MIN_USAGE_PERCENT			 3			// Si una zona esta menos usada que esto, no usarla (pero si todas estan por debajo del threshold, usar la de mayor tamaño)
	#define M_DEBUG						 7			// (DEBUG) Enable debug mode
	#define M_LOGGING					 8			// (DEBUG) Captura backtrace con backtrace() y lo guardas junto con cada allocación.
	#define M_LOGFILE					 9			// (DEBUG) Con diferentes comportamientos según el valor:

	int aprintf(int fd, char const *format, ...);

#pragma endregion

#pragma region "Main"

	int main() {
		// free(): double free detected in tcache 2
		// char popo[] = "hola";
		// free(popo);

		// free(): invalid pointer
		// char *popo = malloc(100);
		// free(&popo[50]);

		// free(): invalid pointer
		// int x = 10;
		// free(&x);

		// invalid size
		// static int x = 10;
		// free(&x);

		// munmap_chunk(): invalid pointer
		// int x = 100;
		// free(&x);

		// kill(getpid(), SIGABRT);		// Abort execution
		// printf("Zone: %u - index: %u\n", get_zonetype(0), get_freelist_index(0));
		// printf("Zone: %u - index: %u\n", get_zonetype(16), get_freelist_index(16));
		// printf("Zone: %u - index: %u\n", get_zonetype(325), get_freelist_index(325));
		// printf("Zone: %u - index: %u\n", get_zonetype(512), get_freelist_index(512));
		// printf("Zone: %u - index: %u\n", get_zonetype(513), get_freelist_index(513));
		// printf("Zone: %u - index: %u\n", get_zonetype(4096), get_freelist_index(4096));
		// printf("Zone: %u - index: %u\n\n", get_zonetype(5000), get_freelist_index(5000));

		options_initialize();
		printf("%s\n", g_manager.options.LOGFILE);

		print_freelist_ranges();
		//while(1);
	}

#pragma endregion

#pragma region "Information"

	// gcc -g -Wno-free-nonheap-object -o testing testing.c -I./inc -L./build/lib -lft_malloc -Wl,-rpath=./build/lib -pthread
	//
	// -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
	// -lft_malloc				= -l busca lib + ft_malloc + .so
	// -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

#pragma endregion
