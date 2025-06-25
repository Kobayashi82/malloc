/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 18:02:43 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/25 13:19:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Validations"

	#pragma region "MXFAST"

		int validate_mxfast(int value) {
			int max = 160;

			if (value < 0)		return (0);
			if (value > max)	return (max);
			return (value);
		}

	#pragma endregion

	#pragma region "FRAGMENTATION_PERCENT"

		int validate_frag_percent(int value) {
			if (value < 0)		return (0);
			if (value > 100)	return (100);
			return (value);
		}

	#pragma endregion

	#pragma region "MIN_ZONE_USAGE_PERCENT"

		int validate_min_usage_percent(int value) {
			if (value < 0)		return (0);
			if (value > 100)	return (100);
			return (value);
		}

	#pragma endregion

	#pragma region "CHECK_ACTION"

		int validate_check_action(int value) {
			if (value < 0)	return (0);
			return (value & 7);
		}

	#pragma endregion

	#pragma region "PERTURB"

		unsigned char validate_perturb(int value) {
			return ((unsigned char)(value & 0xFF));
		}

	#pragma endregion

	#pragma region "ARENA_TEST"

		int validate_arena_test(int value) {
			int max = ARCHITECTURE;

			if (value <= 0)		return (1);
			if (value > max)	return (max);
			return (value);
		}

	#pragma endregion

	#pragma region "ARENA_MAX"

		int validate_arena_max(int value) {
			int max = ARENAS_MAX;

			if ((value <= 0 || value > max) && g_manager.options.ARENA_MAX) return (g_manager.options.ARENA_MAX);
			if (value < 0)		return (0);
			if (value > max)	return (max);
			return (value);
		}

	#pragma endregion

	#pragma region "LOGFILE"

		void validate_logfile(char *value) {
			if (!value || !*value) value = "auto";

			char filename[64] = {0};
			char pid_str[16] = {0};
			pid_t pid = getpid();

			// Default filename with PID
			ft_strlcpy(filename, "malloc_", sizeof(filename));
			ft_itoa_buffered(pid, pid_str, sizeof(pid_str));
			ft_strlcat(filename, pid_str, sizeof(filename));
			ft_strlcat(filename, ".log", sizeof(filename));

			size_t value_len = ft_strlen(value);

			if (!ft_strcmp(value, "auto")) {
				ft_strlcpy(g_manager.options.LOGFILE, "/tmp/", PATH_MAX);			// Temp Directory
				ft_strlcat(g_manager.options.LOGFILE, filename, PATH_MAX);			// Default Filename
			} else if (ft_strchr(value, '/')) {
				ft_strlcpy(g_manager.options.LOGFILE, value, PATH_MAX);				// Directory + Filename
				if (value[value_len - 1] == '/')
					ft_strlcat(g_manager.options.LOGFILE, filename, PATH_MAX);		// Directory + Default Filename
			} else {
				if (!getcwd(g_manager.options.LOGFILE, PATH_MAX))					// Current Directory
					ft_strlcpy(g_manager.options.LOGFILE, "/tmp", PATH_MAX);		// Temp Directory (getcwd failed)
				ft_strlcat(g_manager.options.LOGFILE, "/", PATH_MAX);
				ft_strlcat(g_manager.options.LOGFILE, value, PATH_MAX);				// Filename
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Methods"

	#pragma region "Initialize"

		void options_initialize() {
			char *var = NULL;

			var = getenv("MALLOC_MXFAST_");
			if (var && ft_isdigit_s(var))	g_manager.options.MXFAST = validate_mxfast(ft_atoi(var));
			else							g_manager.options.MXFAST = 80;

			var = getenv("MALLOC_FRAGMENTATION_");
			if (var && ft_isdigit_s(var))	g_manager.options.FRAG_PERCENT = validate_frag_percent(ft_atoi(var));
			else							g_manager.options.FRAG_PERCENT = 75;

			var = getenv("MALLOC_MIN_USAGE_");
			if (var && ft_isdigit_s(var))	g_manager.options.MIN_USAGE_PERCENT = validate_min_usage_percent(ft_atoi(var));
			else							g_manager.options.MIN_USAGE_PERCENT = 10;

			var = getenv("MALLOC_CHECK_");
			if (var && ft_isdigit_s(var))	g_manager.options.CHECK_ACTION = validate_check_action(ft_atoi(var));
			else							g_manager.options.CHECK_ACTION = 0;

			var = getenv("MALLOC_PERTURB_");
			if (var && ft_isdigit_s(var))	g_manager.options.PERTURB = validate_perturb(ft_atoi(var));
			else							g_manager.options.PERTURB = 0;

			var = getenv("MALLOC_ARENA_TEST");
			if (var && ft_isdigit_s(var))	g_manager.options.ARENA_TEST = validate_arena_test(ft_atoi(var));
			else							g_manager.options.ARENA_TEST = ARCHITECTURE == 32 ? 2 : 8;

			var = getenv("MALLOC_ARENA_MAX");
			if (var && ft_isdigit_s(var))	g_manager.options.ARENA_MAX = validate_arena_max(ft_atoi(var));
			else							g_manager.options.ARENA_MAX = 0;

			var = getenv("MALLOC_DEBUG");
			if (var && ft_isdigit_s(var))	g_manager.options.DEBUG = (ft_atoi(var));
			else							g_manager.options.DEBUG = 0;

			var = getenv("MALLOC_LOGGING");
			if (var && ft_isdigit_s(var))	g_manager.options.LOGGING = (ft_atoi(var));
			else							g_manager.options.LOGGING = 0;

			var = getenv("MALLOC_LOGFILE");
			if (var)						validate_logfile(var);
			else							validate_logfile("auto");
		}

	#pragma endregion

	#pragma region "Mallopt"

		__attribute__((visibility("default")))
		int mallopt(int param, int value) {
			ensure_init();

			mutex(&g_manager.mutex, MTX_LOCK);

				if (g_manager.arena_count) {
					if (g_manager.options.DEBUG) aprintf(2, "\t[MALLOPT] Changes are not allowed after the first allocation\n");
					return (1);
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);

			switch (param) {
				case M_MXFAST:				g_manager.options.MXFAST			= validate_mxfast(value);				return (1);
				case M_FRAG_PERCENT:		g_manager.options.FRAG_PERCENT		= validate_frag_percent(value);			return (1);
				case M_MIN_USAGE_PERCENT:	g_manager.options.MIN_USAGE_PERCENT	= validate_min_usage_percent(value);	return (1);
				case M_CHECK_ACTION:		g_manager.options.CHECK_ACTION		= validate_check_action(value);			return (1);
				case M_PERTURB:				g_manager.options.PERTURB			= validate_perturb(value);				return (1);
				case M_ARENA_TEST:			g_manager.options.ARENA_TEST		= validate_arena_test(value);			return (1);
				case M_ARENA_MAX:			g_manager.options.ARENA_MAX			= validate_arena_max(value);			return (1);
				case M_DEBUG:				g_manager.options.DEBUG				= (value);								return (1);
				case M_LOGGING:				g_manager.options.LOGGING			= (value);								return (1);
			}
			return (0);
		}

	#pragma endregion

#pragma endregion
