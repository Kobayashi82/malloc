/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   alloc_mem_ex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:15:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/30 11:41:26 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Print Hex"

	void print_hex(void *ptr, size_t length) {
		if (!ptr || !length) return ;

		unsigned char *tmp_ptr = (unsigned char *)ptr;

		for (size_t i = 0; i < length; i += ALIGNMENT) {
			// Pointer
			aprintf(2, " %p  ", (void *)(tmp_ptr + i));

			// Hexadecimal (8 or 16 bytes per row splitted at 8 or 4 bytes, depends of ALIGNMENT)
			for (size_t j = 0; j < ALIGNMENT; j++) {
				if (i + j < length) {
					if (tmp_ptr[i + j] < 16) aprintf(2, "0");
					aprintf(2, "%x ", (unsigned int)tmp_ptr[i + j]);
				} else aprintf(2, "   ");
				if (j == (ALIGNMENT/2 - 1)) aprintf(2, " ");
			}
			aprintf(2, " ");

			// ASCII
			for (size_t j = 0; j < ALIGNMENT && i + j < length; j++) {
				unsigned char c = tmp_ptr[i + j];
				aprintf(2, "%c", (c >= 32 && c <= 126) ? c : '.');
			}

			aprintf(2, "\n");
		}
	}

#pragma endregion

#pragma region "Show Ex"

	static void show_ex(t_arena *arena, void *ptr, t_heap *heap, size_t offset, size_t length) {
		if (!arena || !ptr || !heap) return ;

		t_chunk *chunk = GET_HEAD(ptr);
		size_t	chunk_size = GET_SIZE(chunk);

		if (offset >= chunk_size) { aprintf(2, "Invalid offset, the maximum offset for this pointer is %u bytes\n", chunk_size - 1);	return ; }

		size_t	remaining = chunk_size - offset;
		size_t	size = (!length || length > remaining) ? remaining : length;

		aprintf(2, "——————————————————————————————————————\n");
		aprintf(2, " • Pointer: %p (Arena #%d)\n", ptr, arena->id);
		aprintf(2, "————————————————————————————————————————————————————————————————————————————————————\n");
		aprintf(2, " • Size: %u bytes      • Offset: %u bytes      • Length: %u bytes%s\n", chunk_size, offset, size, remaining < length ? " (truncated)" : "");
		aprintf(2, "————————————————————————————————————————————————————————————————————————————————————\n");
		print_hex((char *)chunk, sizeof(t_chunk));
		aprintf(2, "————————————————————————————————————————————————————————————————————————————————————\n");
		print_hex((char *)ptr + offset, size);
		aprintf(2, "————————————————————————————————————————————————————————————————————————————————————\n");
	}

#pragma endregion

#pragma region "Validate PTR"

	static int validate_ptr(t_arena *arena, void *ptr, t_heap *heap) {
		if (!arena || !ptr || !heap) return (1);

		// LARGE
		if (heap->type == LARGE) {
			if (GET_HEAD(ptr) == heap->ptr) {
				// Corruption
				if (!HAS_MAGIC(ptr)) { aprintf(2, "Pointer %p is corrupted\n", ptr); return (1); }
				return (0);
			}
			aprintf(2, "Pointer %p is not at the start of a chunk\n", ptr); return (1);
		}

		// Freed
		if (HAS_POISON(ptr)) { aprintf(2, "Pointer %p is not allocated\n", ptr); return (1); }

		// Top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK)) { aprintf(2, "Pointer %p is in top chunk\n", ptr); return (1); }

		// Middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) { aprintf(2, "Pointer %p is not at the start of a chunk\n", ptr); return (1); }

		// Corruption
		if (!HAS_MAGIC(ptr)) { aprintf(2, "Pointer %p is corrupted\n", ptr); return (1); }

		return (0);
	}

#pragma endregion

#pragma region "Show Alloc Mem Ex"

	__attribute__((visibility("default")))
	void show_alloc_mem_ex(void *ptr, size_t offset, size_t length) {
		ensure_init();

		if (!ptr)						{ aprintf(2, "Pointer NULL is invalid\n"); return ; }

		// Not aligned
		if ((uintptr_t)ptr % ALIGNMENT) { aprintf(2, "Pointer %p is not aligned\n", ptr); return ; }

		// malloc(0)
		if (check_digit(ptr, ZERO_MALLOC_BASE)) {
			mutex(&g_manager.mutex, MTX_LOCK);

				if (ptr > ZERO_MALLOC_BASE && ptr < (void *)((char *)ZERO_MALLOC_BASE + (g_manager.zero_malloc_counter * ALIGNMENT)))
					aprintf(2, "Pointer %p is invalid\n", ptr);

			mutex(&g_manager.mutex, MTX_UNLOCK);
			return ;
		}

		t_arena	*arena = &g_manager.arena;
		t_heap	*heap = NULL;

		mutex(&g_manager.mutex, MTX_LOCK);

			while (arena) {
				mutex(&arena->mutex, MTX_LOCK);

					if ((heap = heap_find(arena, ptr))) {
						if (heap->active) {
							if (!validate_ptr(arena, ptr, heap)) show_ex(arena, ptr, heap, offset, length);
							mutex(&arena->mutex, MTX_UNLOCK);
							mutex(&g_manager.mutex, MTX_UNLOCK);
							return ;
						}
					}

				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);

		aprintf(2, "Pointer %p is not allocated\n", ptr); return ;
	}

#pragma endregion

#pragma region "Information"

	// Shows detailed information about an allocated memory pointer with hexadecimal dump.
	//
	//   void show_alloc_mem_ex(void *ptr, size_t offset, size_t length);
	//
	//   ptr    – pointer to allocated memory to examine.
	//   offset – starting byte offset within the chunk (0 = start of user data).
	//   length – number of bytes to display (0 = show from offset to end of chunk).
	//
	//   • On success: displays detailed memory information and hexadecimal dump.
	//   • On failure: prints error message to stderr and returns.
	//
	// Notes:
	//   • Shows chunk header (metadata) and user data separately.
	//   • Automatically truncates length if it exceeds remaining bytes.
	//   • Hexdump format adapts to system alignment (8 or 16 bytes per row).

#pragma endregion
