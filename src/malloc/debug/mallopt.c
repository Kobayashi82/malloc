/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mallopt.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:16:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 19:27:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Mallopt"

	__attribute__((visibility("default")))
	int mallopt(int param, int value) {
		return (options_set(param, value));
	}

#pragma endregion

#pragma region "Information"

	// Adjusts allocator behaviour at run time.
	//
	//   int mallopt(int param, int value);
	//
	//   param – option selector (M_* constant).
	//   value – value assigned to the option.
	//
	//   • On success: returns 1.
	//   • On failure: returns 0 and sets errno to:
	//       – EINVAL: unsupported param or invalid value.
	//
	// Supported params:
	//   • M_ARENA_MAX (-8)       (1-64/128):  Maximum number of arenas allowed.
	//   • M_ARENA_TEST (-7)         (1-160):  Number of arenas at which a hard limit on arenas is computed.
	//   • M_PERTURB (-6)          (0-32/64):  Sets memory to the PERTURB value on allocation, and to value ^ 255 on free.
	//   • M_CHECK_ACTION (-5)         (0-2):  Behaviour on abort errors (0: abort, 1: warning, 2: silence).
	//   • M_MXFAST (1)              (0-160):  Max size (bytes) for fastbin allocations.
	//   • M_MIN_USAGE (3)           (0-100):  Heaps under this usage % are skipped (unless all are under).
	//   • M_DEBUG (7)                 (0-1):  Enables debug mode.
	//   • M_LOGGING (8)               (0-1):  Stores backtrace with each allocation to a file.
	//
	// Notes:
	//   • If both M_DEBUG and M_LOGGING are enabled:
	//       – uses $MALLOC_LOGFILE if defined, or fallback to "/tmp/malloc_[PID].log"

#pragma endregion
