/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 10:21:58 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Can Remove"

	int heap_can_removed(t_arena *arena, t_heap *src_heap) {
		if (!arena) return (0);

		t_heap_header *heap_header = arena->heap_header;
		while (heap_header) {
			t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

			for (int i = 0; i < heap_header->used; ++i) {
				if (heap->active && heap->type == src_heap->type && heap != src_heap) {
					float free_percent = ((float)heap->free / (float)heap->size) * 100.0f;
					float frag_percent = (heap->free_chunks > 1) ? (1.0f - (1.0f / (float)heap->free_chunks)) * 100.0f : 0.0f;
					if (free_percent >= (float)FREE_PERCENT && frag_percent <= (float)FRAG_PERCENT) return (1);
				}
				heap = (t_heap *)((char *)heap + ALIGN(sizeof(t_heap)));
			}

			heap_header = heap_header->next;
		}

		return (0);
	}

#pragma endregion

#pragma region "Find"

	t_heap *heap_find(t_arena *arena, void *ptr) {
		if (!arena || !ptr || !arena->heap_header) return (NULL);

		t_heap *found = NULL;
		t_heap_header *heap_header = arena->heap_header;
		while (heap_header) {
			t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

			for (int i = 0; i < heap_header->used; ++i) {
				size_t padding = (heap->padding >= sizeof(t_chunk)) ? heap->padding - sizeof(t_chunk) : 0;
				if (ptr >= heap->ptr - padding && ptr < (void *)((char *)heap->ptr + heap->size)) {
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

	void *heap_create(t_arena *arena, int type, size_t size, size_t alignment) {
		if (!arena || !size || type < TINY || type > LARGE) return (NULL);

		if		(type == TINY)	size = TINY_SIZE;
		else if (type == SMALL)	size = SMALL_SIZE;
		else					size = (alignment + size + sizeof(t_chunk) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

		void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
		if (ptr == MAP_FAILED) {
			if (print_log(0) && type != LARGE) aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Failed to create heap of size %s (%d)\n", (type == TINY ? "TINY" : "SMALL"), size);
			return (NULL);
		}

		// static int popo = 0;
		// if (type == TINY) {
		// 	aprintf(2, 0, "Creadas: %d\n", popo++);
		// }

		t_heap	*heap = NULL;

		// buscar en las lista si hay un heap con el mismo puntero y longitud igual o menor y usar ese si esta inactivo

		size_t padding = (alignment >= sizeof(t_chunk)) ? alignment - sizeof(t_chunk) : 0;

		if (!arena->heap_header) {
			if (arena == &g_manager.arena) {
				t_heap_header *heap_header = internal_alloc(PAGE_SIZE);
				if (!heap_header) return (NULL);
				arena->heap_header = heap_header;
				heap_header->total = 85;
				heap_header->used = 1;
				heap_header->next = NULL;

				heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
				heap->ptr = (void *)((char *)ptr + padding);
				heap->padding = alignment;
				heap->size = size - alignment;
				heap->free = size - alignment;
				heap->type = type;
				heap->active = true;
				heap->free_chunks = 1;
				heap->top_chunk = heap->ptr;
			} else {
				t_heap_header *heap_header = (t_heap_header *)((char *)arena + ALIGN(sizeof(t_arena)));
				arena->heap_header = heap_header;
				heap_header->total = 73;
				heap_header->used = 1;
				heap_header->next = NULL;

				heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
				heap->ptr = (void *)((char *)ptr + padding);
				heap->padding = alignment;
				heap->size = size - alignment;
				heap->free = size - alignment;
				heap->type = type;
				heap->active = true;
				heap->free_chunks = 1;
				heap->top_chunk = heap->ptr;
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
				new_heap_header->total = 85;
				new_heap_header->used = 1;
				new_heap_header->next = NULL;

				heap = (t_heap *)((char *)new_heap_header + ALIGN(sizeof(t_heap_header)));
				heap->ptr = (void *)((char *)ptr + padding);
				heap->padding = alignment;
				heap->size = size - alignment;
				heap->free = size - alignment;
				heap->type = type;
				heap->active = true;
				heap->free_chunks = 1;
				heap->top_chunk = heap->ptr;
			} else {
				heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));
				heap = (t_heap *)((char *)heap + ((ALIGN(sizeof(t_heap)) * heap_header->used)));
				heap_header->used++;
				heap->ptr = (void *)((char *)ptr + padding);
				heap->padding = alignment;
				heap->size = size - alignment;
				heap->free = size - alignment;
				heap->type = type;
				heap->free_chunks = 1;
				heap->active = true;
				heap->top_chunk = heap->ptr;
			}
		}

		t_chunk *chunk = heap->ptr;
		chunk->size = (heap->size - sizeof(t_chunk)) | PREV_INUSE | (type == SMALL ? HEAP_TYPE : 0) | TOP_CHUNK | (type == LARGE ? MMAP_CHUNK : 0);
		SET_MAGIC(GET_PTR(chunk));
		if (print_log(0) && type != LARGE) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Heap of size %s (%d) allocated\n", heap->ptr, (type == TINY ? "TINY" : "SMALL"), heap->size);

		if (heap && type == LARGE) return (GET_PTR(heap->ptr));

		return ((void *)heap);
	}

#pragma endregion

#pragma region "Destroy"

	int heap_destroy(t_heap *heap) {
		if (!heap) return (1);

		// static int popo = 0;
		// if (heap->type == TINY) {
		// 	aprintf(2, 0, "Eliminadas: %d\n", popo++);
		// }

		size_t padding = (heap->padding >= sizeof(t_chunk)) ? heap->padding - sizeof(t_chunk) : 0;

		int result = 0;
		if (munmap(heap->ptr - padding, heap->size + padding)) {
			result = 1;
			if (print_log(0) && heap->type == LARGE)		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Failed to unmap memory of size %d bytes\n", heap->ptr, heap->size);
			if (print_log(0) && heap->type != LARGE)		aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Failed to detroy heap of size %s (%d)\n", heap->ptr, (heap->type == TINY ? "TINY" : "SMALL"), heap->size);
		}

		heap->active = false;

		if (!result && heap->type == LARGE && print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t   [FREE] Memory freed of size %d bytes\n", heap->ptr, heap->size);
		if (!result && heap->type != LARGE && print_log(0)) aprintf(g_manager.options.fd_out, 1, "%p\t [SYSTEM] Heap of size %s (%d) freed\n", heap->ptr, (heap->type == TINY ? "TINY" : "SMALL"), heap->size);

		return (result);
	}

#pragma endregion

// Arena:						568
// Heap_Header:					16
// Heap:						48

// aprintf(2, 0, "heap: %u\n", ALIGN(sizeof(t_heap)));
