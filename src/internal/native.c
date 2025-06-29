/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   native.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:32:32 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 12:46:37 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Native Free"

	void native_free(void *ptr) {
		if (!ptr) return;

		#ifdef _WIN32
			static void (__cdecl *real_free_win)(void*);
			if (!real_free_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_free_win = (void(__cdecl*)(void*))GetProcAddress(m, "free");
			}

			if (!real_free_win) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to the native free failed\n", ptr);
				return;
			}

			real_free_win(ptr);
		#else
			static void (*real_free_unix)(void*);
			if (!real_free_unix) real_free_unix = dlsym(((void *) -1L), "free");

			if (!real_free_unix) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to the native free failed\n", ptr);
				return;
			}

			real_free_unix(ptr);
		#endif

		if (g_manager.options.DEBUG)			aprintf(g_manager.options.fd_out, "%p\t   [FREE] Delegated to the native free\n", ptr);
	}

#pragma endregion

#pragma region "Native Realloc"

	void *native_realloc(void *ptr, size_t size) {
		if (!ptr || !size) return (NULL);

		#ifdef _WIN32
			static void *(__cdecl *real_realloc_win)(void *, size_t);
			if (!real_realloc_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_realloc_win = (void*(__cdecl*)(void*, size_t))GetProcAddress(m, "realloc");
			}

			if (!real_realloc_win) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to the native realloc failed\n", ptr);
				return (NULL);
			}

			void *new_ptr = real_realloc_win(ptr, size);
		#else
			static void *(*real_realloc_unix)(void *, size_t);
			if (!real_realloc_unix) real_realloc_unix = dlsym(((void *) -1L), "realloc");

			if (!real_realloc_unix) {
				if (g_manager.options.DEBUG)	aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to the native realloc failed\n", ptr);
				return (NULL);
			}

			void *new_ptr = real_realloc_unix(ptr, size);
		#endif

		if (g_manager.options.DEBUG)			aprintf(g_manager.options.fd_out, "%p\t[REALLOC] Delegated to the native realloc from %p with %d bytes\n", new_ptr, ptr, size);
		return (new_ptr);
	}

#pragma endregion

#pragma region "Native Malloc Usable Size"

	size_t native_malloc_usable_size(void *ptr) {
		if (!ptr) return 0;

		#ifdef _WIN32
			static size_t (__cdecl *real_malloc_usable_size_win)(void*);
			if (!real_malloc_usable_size_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_malloc_usable_size_win = (size_t(__cdecl*)(void*))GetProcAddress(m, "_msize");
			}

			if (!real_malloc_usable_size_win) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to native malloc_usable_size failed\n", ptr);
				return (0);
			}

			return (real_malloc_usable_size_win(ptr));
		#else
			static size_t (*real_malloc_usable_size_unix)(void*);
			if (!real_malloc_usable_size_unix) 
				real_malloc_usable_size_unix = dlsym(RTLD_NEXT, "malloc_usable_size");
			
			if (!real_malloc_usable_size_unix) {
				if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Delegation to native malloc_usable_size failed\n", ptr);
				return (0);
			}

			return (real_malloc_usable_size_unix(ptr));
		#endif
	}

#pragma endregion
