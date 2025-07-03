/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/25 18:02:43 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 13:12:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "internal.h"

#pragma endregion

#pragma region "Validate"

	#pragma region "MXFAST"

		static int validate_mxfast(int value) {
			if (value < 0 || value > 160) return (0);

			g_manager.options.MXFAST = value;

			return (1);
		}

	#pragma endregion

	#pragma region "MIN_ZONE_USAGE"

		static int validate_min_usage(int value) {
			if (value < 0 || value > 100) return (0);

			g_manager.options.MIN_USAGE = value;

			return (1);
		}

	#pragma endregion

	#pragma region "CHECK_ACTION"

		static int validate_check_action(int value) {
			if (value < 0 || value > 3) return (0);

			g_manager.options.CHECK_ACTION = value;

			return (1);
		}

	#pragma endregion

	#pragma region "PERTURB"

		static unsigned char validate_perturb(int value) {
			if (value < 0 || value > 255) return (0);

			g_manager.options.PERTURB = value;

			return (1);
		}

	#pragma endregion

	#pragma region "ARENA_TEST"

		static int validate_arena_test(int value) {
			if (value <= 0 || value > (int)ARCHITECTURE) return (0);

			g_manager.options.ARENA_TEST = value;

			return (1);
		}

	#pragma endregion

	#pragma region "ARENA_MAX"

		static int validate_arena_max(int value) {
			if (value <= 0 || value > (int)ARCHITECTURE * 2) return (0);

			g_manager.options.ARENA_MAX = value;

			return (1);
		}

	#pragma endregion

	#pragma region "DEBUG"

		static int validate_debug(int value) {
			if (value <= 0 || value > 1) return (0);

			g_manager.options.DEBUG = value;

			return (1);
		}

	#pragma endregion

	#pragma region "LOGGING"

		static int validate_logging(int value) {
			if (value <= 0 || value > 2) return (0);

			g_manager.options.LOGGING = value;

			return (1);
		}

	#pragma endregion

	#pragma region "LOGFILE"

		static int validate_logfile(char *value) {
			if (!value || !*value) value = "auto";

			char pid_str[21] = {0};
			pid_t pid = getpid();

			pid_str[0] = '_';
			ft_itoa_buffered(pid, pid_str + 1, sizeof(pid_str) - 1);
			ft_strlcat(pid_str, ".log", sizeof(pid_str));

			size_t value_len = ft_strlen(value);

			if (!ft_strcmp(value, "auto")) {
				ft_strlcpy(g_manager.options.LOGFILE, "/tmp/malloc", PATH_MAX);
				ft_strlcat(g_manager.options.LOGFILE, pid_str, PATH_MAX);
			} else if (ft_strchr(value, '/')) {
				ft_strlcpy(g_manager.options.LOGFILE, value, PATH_MAX);
				if (value[value_len - 1] == '/') {
					ft_strlcat(g_manager.options.LOGFILE, "malloc", PATH_MAX);
					ft_strlcat(g_manager.options.LOGFILE, pid_str, PATH_MAX);
				}
			} else {
				if (!getcwd(g_manager.options.LOGFILE, PATH_MAX))
					ft_strlcpy(g_manager.options.LOGFILE, "/tmp", PATH_MAX);
				ft_strlcat(g_manager.options.LOGFILE, "/", PATH_MAX);
				ft_strlcat(g_manager.options.LOGFILE, value, PATH_MAX);
				ft_strlcat(g_manager.options.LOGFILE, pid_str, PATH_MAX);
			}

			g_manager.options.fd_out = open(g_manager.options.LOGFILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
			if (g_manager.options.fd_out == -1) {
				if (g_manager.options.DEBUG) aprintf(2, 1, "\t\t  [ERROR] Unable to create log, falling back to default location '/tmp/malloc_[PID].log'\n");
				ft_strlcpy(g_manager.options.LOGFILE, "/tmp/malloc", PATH_MAX);
				ft_strlcat(g_manager.options.LOGFILE, pid_str, PATH_MAX);
				g_manager.options.fd_out = open(g_manager.options.LOGFILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
			}

			return (1);
		}

	#pragma endregion

#pragma endregion

#pragma region "Initialize"

	void options_initialize() {
		char *var = NULL;

		g_manager.options.fd_out = 2;

		var = getenv("MALLOC_MXFAST_");
		if (var && ft_isdigit_s(var))	validate_mxfast(ft_atoi(var));
		else							g_manager.options.MXFAST = 80;

		var = getenv("MALLOC_MIN_USAGE_");
		if (var && ft_isdigit_s(var))	validate_min_usage(ft_atoi(var));
		else							g_manager.options.MIN_USAGE = 10;

		var = getenv("MALLOC_CHECK_");
		if (var && ft_isdigit_s(var))	validate_check_action(ft_atoi(var));
		else							g_manager.options.CHECK_ACTION = 0;

		var = getenv("MALLOC_PERTURB_");
		if (var && ft_isdigit_s(var))	validate_perturb(ft_atoi(var));
		else							g_manager.options.PERTURB = 0;

		var = getenv("MALLOC_ARENA_TEST");
		if (var && ft_isdigit_s(var))	validate_arena_test(ft_atoi(var));
		else							g_manager.options.ARENA_TEST = ARCHITECTURE == 32 ? 2 : 8;

		var = getenv("MALLOC_ARENA_MAX");
		if (var && ft_isdigit_s(var))	validate_arena_max(ft_atoi(var));
		else							g_manager.options.ARENA_MAX = 0;

		var = getenv("MALLOC_DEBUG");
		if (var && ft_isdigit_s(var))	validate_debug(ft_atoi(var));
		else							g_manager.options.DEBUG = 0;

		var = getenv("MALLOC_LOGGING");
		if (var && ft_isdigit_s(var))	validate_logging(ft_atoi(var));
		else							g_manager.options.LOGGING = 0;

		if (g_manager.options.LOGGING == 1) {
			var = getenv("MALLOC_LOGFILE");
			if (var)					validate_logfile(var);
			else						validate_logfile("auto");
		}
	}

#pragma endregion

#pragma region "Set"

	int options_set(int param, int value) {
		ensure_init();

		mutex(&g_manager.mutex, MTX_LOCK);

			if (g_manager.arena_count) {
				if (print_log(0)) aprintf(g_manager.options.fd_out, 1, "\t[MALLOPT] Changes are not allowed after the first allocation\n");
				errno = EINVAL;
				return (0);
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);

		int result = 0;
		switch (param) {
			case M_MXFAST:			result = validate_mxfast(value);		break;
			case M_MIN_USAGE:		result = validate_min_usage(value);		break;
			case M_CHECK_ACTION:	result = validate_check_action(value);	break;
			case M_PERTURB:			result = validate_perturb(value);		break;
			case M_ARENA_TEST:		result = validate_arena_test(value);	break;
			case M_ARENA_MAX:		result = validate_arena_max(value);		break;
			case M_DEBUG:			result = validate_debug(value);			break;
			case M_LOGGING:			result = validate_logging(value);		break;
		}

		if (param == M_DEBUG || param == M_LOGGING) {
			if (g_manager.options.LOGGING == 1 && !*g_manager.options.LOGFILE) {
				char *var = getenv("MALLOC_LOGFILE");
				if (var)	validate_logfile(var);
			}
		}

		if (result) errno = EINVAL;
		return (result);
	}

#pragma endregion
