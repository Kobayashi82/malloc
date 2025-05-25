/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 18:02:43 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/25 19:24:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "malloc.h"

#pragma endregion

int validate_mxfast(int value) {
	int max = (80 * sizeof(size_t)) / 4;

	if (g_arena_manager.first_alloc) return (g_arena_manager.options.MXFAST);

	if (value < 0)		return (0);
	if (value > max)	return (max);
	return (value);
}

void mallopt_init() {
	char *var = NULL;

	var = getenv("MALLOC_MXFAST_");
	if (var && ft_isdigit_s(var))	g_arena_manager.options.MXFAST = validate_mxfast(ft_atoi(var));
	else							g_arena_manager.options.MXFAST = (64 * sizeof(size_t)) / 4;

	var = getenv("MALLOC_TRIM_THRESHOLD_");
	if (var && ft_isdigit_s(var))	g_arena_manager.options.TRIM_THRESHOLD = validate_mxfast(ft_atoi(var));
	else							g_arena_manager.options.TRIM_THRESHOLD = (64 * sizeof(size_t)) / 4;

	g_arena_manager.options.TOP_PAD				= ft_atoi(getenv("MALLOC_TOP_PAD_"));
	g_arena_manager.options.MMAP_THRESHOLD		= ft_atoi(getenv("MALLOC_MMAP_THRESHOLD_"));
	g_arena_manager.options.MMAP_MAX			= ft_atoi(getenv("MALLOC_MMAP_MAX_"));
	g_arena_manager.options.CHECK_ACTION		= ft_atoi(getenv("MALLOC_CHECK_"));
	g_arena_manager.options.PERTURB				= ft_atoi(getenv("MALLOC_PERTURB_"));
	g_arena_manager.options.ARENA_TEST			= ft_atoi(getenv("MALLOC_ARENA_TEST"));
	g_arena_manager.options.ARENA_MAX			= ft_atoi(getenv("MALLOC_ARENA_MAX"));
}

int mallopt(int param, int value) {
	switch (param) {
		case M_MXFAST:			g_arena_manager.options.MXFAST = validate_mxfast(value);		break;
		case M_TRIM_THRESHOLD:	g_arena_manager.options.TRIM_THRESHOLD = value;					break;
		case M_TOP_PAD:			g_arena_manager.options.TOP_PAD = value;						break;
		case M_MMAP_THRESHOLD:	g_arena_manager.options.MMAP_THRESHOLD = value;					break;
		case M_MMAP_MAX:		g_arena_manager.options.MMAP_MAX = value;						break;
		case M_CHECK_ACTION:	g_arena_manager.options.CHECK_ACTION = value;					break;
		case M_PERTURB:			g_arena_manager.options.PERTURB = value;						break;
		case M_ARENA_TEST:		g_arena_manager.options.ARENA_TEST = value;						break;
		case M_ARENA_MAX:		g_arena_manager.options.ARENA_MAX = value;						break;
	}
}
