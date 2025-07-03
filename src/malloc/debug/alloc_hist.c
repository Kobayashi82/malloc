/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   alloc_hist.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:16:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 14:01:48 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Hist Extend"

	void heap_hist_extend() {
		if (!g_manager.hist_size) {
			g_manager.hist_size = PAGE_SIZE;
			void *ptr = mmap(NULL, g_manager.hist_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
			if (ptr == MAP_FAILED) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 0, "\t\t  [ERROR] Failed to create heap for allocation history\n");
				g_manager.hist_size = SIZE_MAX;
				return ;
			}
			g_manager.hist_buffer = ptr;
		} else if (g_manager.hist_buffer && g_manager.hist_size < SIZE_MAX) {
			size_t old_size = g_manager.hist_size;
			g_manager.hist_size *= 2;
			void *ptr = mmap(NULL, g_manager.hist_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
			if (ptr == MAP_FAILED) {
				if (print_log(1))		aprintf(g_manager.options.fd_out, 0, "\t\t  [ERROR] Failed to create heap for allocation history\n");
				g_manager.hist_size = SIZE_MAX;
				if (munmap(g_manager.hist_buffer, g_manager.hist_size))
					if (print_log(1))	aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Failed to unmap memory for allocation history\n", g_manager.hist_buffer);
				return ;
			}

			ft_memcpy(ptr, g_manager.hist_buffer, g_manager.hist_pos);
			if (munmap(g_manager.hist_buffer, old_size))
				if (print_log(1))		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Failed to unmap memory for allocation history\n", g_manager.hist_buffer);

			g_manager.hist_buffer = ptr;
		}
	}

#pragma endregion

#pragma region "Hist Destroy"

	void heap_hist_destroy() {
		mutex(&g_manager.mutex, MTX_LOCK);

			if (g_manager.hist_buffer && g_manager.hist_size && g_manager.hist_size < SIZE_MAX) {
				if (munmap(g_manager.hist_buffer, g_manager.hist_size))
					if (print_log(1)) 	aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Failed to unmap memory for allocation history\n", g_manager.hist_buffer);
				g_manager.hist_buffer = NULL;
				g_manager.hist_size = 0;
				g_manager.hist_pos = 0;
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);
	}

#pragma endregion

#pragma region "Show Alloc History"

	__attribute__((visibility("default")))
	void show_alloc_history() {
		if (print_log(0)) {
			mutex(&g_manager.hist_mutex, MTX_LOCK);

				if (g_manager.hist_buffer && g_manager.hist_size && g_manager.hist_size < SIZE_MAX)
					write(2, g_manager.hist_buffer, g_manager.hist_pos);

			mutex(&g_manager.hist_mutex, MTX_UNLOCK);
		}
	}

#pragma endregion

#pragma region "Information"

	// Displays the internal allocation history log.
	//
	//   void show_alloc_history(void);
	//
	//   • On call: writes the recorded log buffer to file descriptor 2 (stderr).
	//   • The log contains a chronological trace of:
	//       – Allocations, frees and errors.

#pragma endregion
