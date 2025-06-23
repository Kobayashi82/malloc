/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:33:27 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/24 01:40:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Real Free"

	void realfree(void *ptr) {
		#ifdef _WIN32
			static void (__cdecl *real_free_win)(void*);
			if (!real_free_win) {
				HMODULE m = GetModuleHandleA("msvcrt.dll");
				if (m) real_free_win = (void(__cdecl*)(void*))GetProcAddress(m, "free");
			}
			if (!ptr) return;

			if (!real_free_win) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate free to native allocator\n", ptr);	
				return;
			}

			if (g_manager.options.DEBUG) aprintf(1, "%p\t   [FREE] Delegated to the native allocator\n", ptr);
			real_free_win(ptr);
		#else
			static void (*real_free_unix)(void*);
			if (!real_free_unix) real_free_unix = dlsym(((void *) -1L), "free");
			if (!ptr) return;

			if (!real_free_unix) {
				if (g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Failed to delegate free to native allocator\n", ptr);
				return;
			}

			if (g_manager.options.DEBUG) aprintf(1, "%p\t   [FREE] Delegated to the native allocator\n", ptr);
			real_free_unix(ptr);
		#endif
	}

#pragma endregion

#pragma region "Free PTR"

	static int free_ptr(void *ptr, t_arena *arena, t_heap *heap) {
		if (!ptr ||!arena || !heap) return (0);
		
		// Not aligned
		if (!IS_ALIGNED(ptr)) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (not aligned)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		// Heap freed
		if (!heap->active) {
			if (heap->type == LARGE) {
				if ((void *)((char *)heap->ptr + sizeof(t_lchunk)) == ptr) {
					if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Double free\n", ptr);
					else							aprintf(1, "Double free\n");
				} else {
					if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
					else							aprintf(1, "Invalid pointer\n");			
				}
			} else {
				if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
				else							aprintf(1, "Invalid pointer\n");
			}
			abort();
		}

		// LARGE
		if (heap->type == LARGE) {
			if ((void *)((char *)(ptr) - sizeof(t_lchunk)) == heap->ptr) {
				int result = heap_destroy(heap->ptr, heap->size, LARGE);
				if (result && g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Unable to unmap memory\n", ptr);
				else if (g_manager.options.DEBUG)		aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);
				return (0);
			}

			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		// Double free
		if (HAS_POISON(ptr)) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Double free\n", ptr);
			else							aprintf(1, "Double free\n");
			abort();
		}

		// In top chunk
		t_chunk *chunk = (t_chunk *)GET_HEAD(ptr);
		if ((chunk->size & TOP_CHUNK)) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in top chunk)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		// In middle chunk
		t_chunk *next_chunk = GET_NEXT(chunk);
		if (!(next_chunk->size & PREV_INUSE)) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t  [ERROR] Invalid pointer (in middle of chunk)\n", ptr);
			else							aprintf(1, "Invalid pointer\n");
			abort();
		}

		SET_POISON(ptr);
		heap->free += GET_SIZE(chunk) + sizeof(t_chunk);

		// Add to bins
		next_chunk->size &= ~PREV_INUSE;
		t_chunk_int chunk_size = GET_SIZE(chunk) + sizeof(t_chunk);
		if (chunk_size <= (t_chunk_int)g_manager.options.MXFAST) {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t [SYSTEM] Chunk added to FastBin\n", chunk);
			link_chunk(chunk, chunk_size, FASTBIN, arena);
		} else {
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t [SYSTEM] Coalescing adjacent chunks\n", chunk);
			chunk = coalescing(chunk, arena, heap);
			if (g_manager.options.DEBUG)	aprintf(1, "%p\t [SYSTEM] Chunk added to UnsortedBin\n", chunk);
			link_chunk(chunk, chunk_size, UNSORTEDBIN, arena);
		}

		arena->free_count++;
		if (heap->free == heap->size) {
			// Mark for elimination
		}

		if (g_manager.options.DEBUG)	aprintf(1, "%p\t   [FREE] Memory freed of size %d bytes\n", ptr, chunk_size);

		return (0);
	}
	
#pragma endregion

	int	first_digit(void *ptr) {
		uintptr_t val = (uintptr_t)ptr;

		while (val >= 0x10) val /= 0x10;
		return ((int)(val & 0xF));
	}

	int	check_digit(t_arena *arena, void *ptr1) {
		int	target_digit = first_digit(ptr1);
		int	base_digit = 0;

		if		(arena->tiny) base_digit = first_digit(arena->tiny);
		else if (arena->tiny) base_digit = first_digit(arena->small);
		else if (arena->tiny) base_digit = first_digit(arena->large);

		return (base_digit && target_digit == base_digit);
	}

#pragma region "Free"

	__attribute__((visibility("default")))
	void free(void *ptr) {
		ensure_init();

		if (!ptr) return;

		t_arena	*arena = tcache;
		t_heap	*heap_ptr = NULL;
		
		if (!arena) arena = &g_manager.arena;
		if (arena) {
			mutex(&arena->mutex, MTX_LOCK);

				if ((heap_ptr = heap_find(ptr, arena))) free_ptr(ptr, arena, heap_ptr);
				
			mutex(&arena->mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) {
			mutex(&g_manager.mutex, MTX_LOCK);

				t_arena *curr_arena = &g_manager.arena;
				while (curr_arena) {
					if (arena == curr_arena) { curr_arena = curr_arena->next; continue; }
					mutex(&curr_arena->mutex, MTX_LOCK);

						if ((heap_ptr = heap_find(ptr, curr_arena))) free_ptr(ptr, curr_arena, heap_ptr);

					mutex(&curr_arena->mutex, MTX_UNLOCK);
					if (heap_ptr) break;
					curr_arena = curr_arena->next;
				}

			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

		if (!heap_ptr) {
			if (!check_digit(&g_manager.arena, ptr))	realfree(ptr);
			else if (g_manager.options.DEBUG)			aprintf(1, "%p\t  [ERROR] Invalid pointer (not allocated)\n", ptr);
			else										aprintf(1, "Invalid pointer\n");
		}
	}

#pragma endregion
