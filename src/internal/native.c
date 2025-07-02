/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   native.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:32:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 08:26:44 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Native Free"

	void native_free(void *ptr) {
		if (!ptr) return;

		static void (*real_free_unix)(void*);
		if (!real_free_unix) real_free_unix = dlsym(((void *) -1L), "free");

		if (!real_free_unix) {
			if (print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Delegation to the native free failed\n", ptr);
			return;
		}

		real_free_unix(ptr);
		if (print_log(0))		aprintf(g_manager.options.fd_out, 1, "%p\t   [FREE] Delegated to the native free\n", ptr);
	}

#pragma endregion

#pragma region "Native Realloc"

	void *native_realloc(void *ptr, size_t size) {
		if (!ptr || !size) return (NULL);

		static void *(*real_realloc_unix)(void *, size_t);
		if (!real_realloc_unix) real_realloc_unix = dlsym(((void *) -1L), "realloc");

		if (!real_realloc_unix) {
			if (print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Delegation to the native realloc failed\n", ptr);
			return (NULL);
		}

		void *new_ptr = real_realloc_unix(ptr, size);

		if (print_log(0))		aprintf(g_manager.options.fd_out, 1, "%p\t[REALLOC] Delegated to the native realloc from %p with %d bytes\n", new_ptr, ptr, size);
		return (new_ptr);
	}

#pragma endregion

#pragma region "Native Malloc Usable Size"

	size_t native_malloc_usable_size(void *ptr) {
		if (!ptr) return 0;

		static size_t (*real_malloc_usable_size_unix)(void*);
		if (!real_malloc_usable_size_unix) real_malloc_usable_size_unix = dlsym(RTLD_NEXT, "malloc_usable_size");

		if (!real_malloc_usable_size_unix) {
			if (print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Delegation to native malloc_usable_size failed\n", ptr);
			return (0);
		}

		if (print_log(0))		aprintf(g_manager.options.fd_out, 1, "%p\t[MALLOC_USABLE_SIZE] Delegated to the native malloc_usable_size\n");
		return (real_malloc_usable_size_unix(ptr));
	}

#pragma endregion
