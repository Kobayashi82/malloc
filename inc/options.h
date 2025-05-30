/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 09:14:48 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/30 13:40:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <time.h>
	#include <stdbool.h>
	#include <dlfcn.h>

	#ifdef _WIN32
		#include <windows.h>
	#endif

#pragma endregion

#pragma region "Variables"

	#pragma region "Defines"

		#define ARCHITECTURE				32 * ((sizeof(long) != 4) + 1)	// 32 or 64 bits
		#define PAGE_SIZE					get_pagesize();					// 4096
		#define ARENAS_MAX					2 * ARCHITECTURE				// 64 or 128
		#define HEAPS_MAX					4 * ARCHITECTURE				// 128 or 256
		#define INVALID_INDEX				~(unsigned char)0				// 255

		#ifndef SIZE_MAX
			#define SIZE_MAX				~(size_t)0
		#endif
		#ifndef PATH_MAX
			#define PATH_MAX 				4096
		#endif

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

		#define FREELIST_SIZE	32						// MAX = INVALID_INDEX

	#pragma endregion

	#pragma region "Structures"

		typedef struct s_options {
			int				MXFAST;					// Max size (in bytes) for allocations using fastbins (0 disables fastbins)
			int				FRAG_PERCENT;			// Si una zona esta mas fragmentada que esto, no usarla (crear nueva si es necesario)
			int				MIN_USAGE_PERCENT;		// Si una zona esta menos usada que esto, no usarla (pero si todas estan por debajo del threshold, usar la de mayor tamaño)
			int				CHECK_ACTION;			// Muestra informacion cuando hay errores
			unsigned char	PERTURB;				// Rellena la zona de memoria. malloc = ~PERTURB	free = PERTURB
			int				ARENA_TEST;				// 
			int				ARENA_MAX;				// Hard limit for arenas based on CPU count
			bool			DEBUG;
			bool			LOGGING;
			char 			LOGFILE[PATH_MAX];
		} t_options;

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	size_t	get_pagesize();
	int		mallopt(int param, int value);
	void	options_initialize();

#pragma endregion
