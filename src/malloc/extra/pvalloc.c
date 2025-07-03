/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pvalloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 22:43:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 20:55:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Pvalloc"

	__attribute__((visibility("default")))
	void *pvalloc(size_t size) {
		ensure_init();

		void *ptr = NULL;

		if (size > SIZE_MAX - sizeof(t_chunk)) { errno = ENOMEM; return (NULL); }
		size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

		if (!size) {
			mutex(&g_manager.mutex, MTX_LOCK);

				size_t aligned_offset = (g_manager.alloc_zero_counter * PAGE_SIZE);
				g_manager.alloc_zero_counter++;
				
			mutex(&g_manager.mutex, MTX_UNLOCK);

			ptr = (void*)(ZERO_MALLOC_BASE + aligned_offset);
			if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [VALLOC] Allocated %u bytes\n", ptr, size);
			else if (!ptr) errno = ENOMEM;

			return (ptr);
		}

		ptr = allocate_aligned("PVCALLOC", PAGE_SIZE, size);

		if (ptr && print_log(0))	aprintf(g_manager.options.fd_out, 1, "%p\t [PVALLOC] Allocated %u bytes\n", ptr, size);
		if (!ptr && print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to allocated %u bytes\n", size);

		if (ptr) SET_MAGIC(ptr);
		else errno = ENOMEM;

		return (ptr);
	}

#pragma endregion

#pragma region "Information"

	// Allocates a page‑aligned block rounded up to a full page.
	//
	//   void *pvalloc(size_t size);
	//
	//   size – the size of the memory block to allocate, in bytes. Rounds up to the next multiple of page size
	//
	//   • On success: returns a pointer aligned to the system page size.
	//   • On failure: returns NULL and sets errno to:
	//       – ENOMEM: not enough memory.
	//
	// Notes:
	//   • pvalloc() is non‑standard and obsolete; prefer posix_memalign() or aligned_alloc() for portable code.

#pragma endregion
