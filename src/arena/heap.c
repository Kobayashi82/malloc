/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/27 23:59:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Find"

	t_heap *heap_find(t_arena *arena, void *ptr) {
		if (!arena || !ptr || !arena->heap_header) return (NULL);

		t_heap *found = NULL;
		t_heap_header *heap_header = arena->heap_header;
		while (heap_header) {
			t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

			for (int i = 0; i < heap_header->used; ++i) {
				if (ptr >= heap->ptr && ptr < (void *)((char *)heap->ptr + heap->size)) {
					if (heap->active) return (heap);
					found = heap;
				}
				heap = (t_heap *)((char *)heap + ALIGN(sizeof(t_heap)));
			}

			heap_header = heap_header->next;
		}

		return (found);
	}

#pragma endregion

#pragma region "Create"

	void *heap_create(t_arena *arena, int type, size_t size) {
		if (!arena || !size || type < TINY || type > LARGE) return (NULL);

		if		(type == TINY)	size = TINY_SIZE;
		else if (type == SMALL)	size = SMALL_SIZE;
		else					size = (size + sizeof(t_chunk) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
		void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
		if (ptr == MAP_FAILED) {
			if (g_manager.options.DEBUG && type != LARGE) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to create heap of size %s (%d)\n", (type == TINY ? "TINY" : "SMALL"), size);
			return (NULL);
		}

		t_heap	*heap = NULL;

		// buscar en las lista si hay un heap con el mismo puntero y longitud igual o menor y usar ese si esta inactivo

		if (!arena->heap_header) {
			if (arena == &g_manager.arena) {
				t_heap_header *heap_header = internal_alloc(PAGE_SIZE);
				if (!heap_header) return (NULL);
				arena->heap_header = heap_header;
				heap_header->total = 60;
				heap_header->used = 1;
				heap_header->next = NULL;

				heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
				heap->ptr = ptr;
				heap->size = size;
				heap->free = size - sizeof(t_chunk);
				heap->type = type;
				heap->active = true;
				heap->top_chunk = ptr;
			} else {
				t_heap_header *heap_header = (t_heap_header *)((char *)arena + ALIGN(sizeof(t_arena)));
				arena->heap_header = heap_header;
				heap_header->total = 50;
				heap_header->used = 1;
				heap_header->next = NULL;

				heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
				heap->ptr = ptr;
				heap->size = size;
				heap->free = size - sizeof(t_chunk);
				heap->type = type;
				heap->active = true;
				heap->top_chunk = ptr;
			}
		} else {
			bool found = false;
			t_heap_header *heap_header = arena->heap_header;
			while (heap_header) {
				if (heap_header->used < heap_header->total) { found = true; break; }
				if (!heap_header->next) break;
				heap_header = heap_header->next;
			}

			if (!found) {
				t_heap_header *new_heap_header = internal_alloc(PAGE_SIZE);
				if (!heap_header) return (NULL);
				heap_header->next = new_heap_header;
				new_heap_header->total = 60;
				new_heap_header->used = 1;
				new_heap_header->next = NULL;

				heap = (t_heap *)((char *)new_heap_header + ALIGN(sizeof(t_heap_header)));
				heap->ptr = ptr;
				heap->size = size;
				heap->free = size - sizeof(t_chunk);
				heap->type = type;
				heap->active = true;
				heap->top_chunk = ptr;
			} else {
				heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
				heap = (t_heap *)((char *)heap + ((ALIGN(sizeof(t_heap)) * heap_header->used)));
				heap_header->used++;
				heap->ptr = ptr;
				heap->size = size;
				heap->free = size - sizeof(t_chunk);
				heap->type = type;
				heap->active = true;
				heap->top_chunk = ptr;
			}
		}

		t_chunk *chunk = (t_chunk *)ptr;
		chunk->size = (size - sizeof(t_chunk)) | PREV_INUSE | (type == SMALL ? HEAP_TYPE : 0) | TOP_CHUNK | (type == LARGE ? MMAP_CHUNK : 0);

		if (g_manager.options.DEBUG && type != LARGE) aprintf(g_manager.options.fd_out, "%p\t [SYSTEM] Heap of size %s (%d) allocated\n", heap->ptr, (type == TINY ? "TINY" : "SMALL"), heap->size);

		if (heap && type == LARGE) {
			arena->alloc_count++;
			return (GET_PTR(heap->ptr));
		}
	
		// static int p = 0;
		// aprintf(1, "%p heap %d - %d\n", arena, p++, size);

		return ((void *)heap);
	}

#pragma endregion

#pragma region "Destroy"

	int heap_destroy(t_arena *arena, void *ptr, int type, size_t size) {
		if (!arena || !ptr || !size || type < TINY || type > LARGE) return (1);
	
		t_heap *heap = heap_find(arena, ptr);
		if (!heap) {
			if (g_manager.options.DEBUG && type != LARGE)		aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to detroy heap\n");
			return (1);
		}

		int result = 0;
		if (munmap(ptr, size)) {
			result = 1;
			if (g_manager.options.DEBUG && type != LARGE)		aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Failed to detroy heap\n");
		}
		heap->active = false;

		if (!result && type != LARGE && g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t [SYSTEM] Heap of size (%d) freed\n", ptr, size);
		if (!result && type == LARGE) arena->free_count++;

		return (result);
	}

#pragma endregion
