/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/26 00:24:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Heap"

	#pragma region "Find"

		t_iheap *heap_find(void *ptr, t_arena *arena) {
			if (!ptr || !arena) return (NULL);

			// t_hiheap *curr = arena->hiheap;
			// while (curr) {
			// 	if (ptr >= curr->ptr && ptr <= (void *)((char *)curr->ptr + curr->size)) return (curr);
			// 	curr = curr->next;
			// }

			return (NULL);
		}

	#pragma endregion

	#pragma region "Create"

		#pragma region "Allocate"

			static t_iheap *heap_allocate(t_arena *arena, size_t size, e_heaptype type) {
				if (!size || type < 0 || type > 2) {
					if (g_manager.options.DEBUG && type != LARGE)		aprintf(2, "\t\t  [ERROR] Failed to create heap\n", size);
					return (NULL);
				}

				void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
				if (ptr == MAP_FAILED) {
					if (g_manager.options.DEBUG && type != LARGE)		aprintf(2, "\t\t  [ERROR] Failed to create heap\n", size);
					return (NULL);
				}

				t_iheap	*iheap = NULL;

				if (!arena->hiheap) {
					if (arena == &g_manager.arena) {
						t_hiheap *hiheap = internal_alloc(PAGE_SIZE);
						arena->hiheap = hiheap;
						hiheap->total = 60;
						hiheap->used = 1;
						hiheap->next = NULL;

						iheap = (t_iheap *)((char *)hiheap + ALIGN(sizeof(t_hiheap)));
						iheap->ptr = ptr;
						iheap->size = size;
						iheap->free = size - sizeof(t_chunk);
						iheap->type = type;
						iheap->active = true;
						iheap->top_chunk = ptr;
					} else {
						t_hiheap *hiheap = (t_hiheap *)((char *)arena + ALIGN(sizeof(t_arena)));
						arena->hiheap = hiheap;
						hiheap->total = 50;
						hiheap->used = 1;
						hiheap->next = NULL;

						iheap = (t_iheap *)((char *)hiheap + ALIGN(sizeof(t_hiheap)));
						iheap->ptr = ptr;
						iheap->size = size;
						iheap->free = size - sizeof(t_chunk);
						iheap->type = type;
						iheap->active = true;
						iheap->top_chunk = ptr;
					}
				} else {
					t_hiheap *hiheap = arena->hiheap;
					while (!hiheap) {
						if (hiheap->used < hiheap->total) break;
						if (hiheap->next) hiheap = hiheap->next;
					}

					if (!hiheap) {
						t_hiheap *new_hiheap = internal_alloc(PAGE_SIZE);
						hiheap->next = new_hiheap;
						new_hiheap->total = 60;
						new_hiheap->used = 1;
						new_hiheap->next = NULL;

						iheap = (t_iheap *)((char *)hiheap + ALIGN(sizeof(t_hiheap)));
						iheap->ptr = ptr;
						iheap->size = size;
						iheap->free = size - sizeof(t_chunk);
						iheap->type = type;
						iheap->active = true;
						iheap->top_chunk = ptr;
					} else {
						iheap = (t_iheap *)((char *)hiheap + ALIGN(sizeof(t_hiheap)));
						for (int i = 0; i < hiheap->used; ++i) iheap++;
						hiheap->used++;
						iheap->ptr = ptr;
						iheap->size = size;
						iheap->free = size - sizeof(t_chunk);
						iheap->type = type;
						iheap->active = true;
						iheap->top_chunk = ptr;
					}
				}

				if (type == LARGE) {
					t_chunk *chunk = (t_chunk *)ptr;
					chunk->prev_size = 0;
					chunk->size = (size - sizeof(t_chunk)) | PREV_INUSE | TOP_CHUNK;
					ptr = (void *)((char *)chunk + sizeof(t_chunk));
				} else {
					t_chunk *chunk = (t_chunk *)ptr;
					chunk->prev_size = 0;
					chunk->size = (size - sizeof(t_chunk)) | PREV_INUSE | (type == SMALL ? HEAP_TYPE : 0) | TOP_CHUNK;
					ptr = (void *)((char *)chunk + sizeof(t_chunk));
				}

				if (g_manager.options.DEBUG && type != LARGE)			aprintf(2, "%p\t [SYSTEM] Heap of size (%d) allocated\n", iheap->ptr, iheap->size);

				return (iheap);
			}

		#pragma endregion

		#pragma region "Create"

			void *heap_create(t_arena *arena, e_heaptype type, size_t size) {
				if (!tcache || !size || type < 0 || type > 2) return (NULL);

				t_iheap	*iheap = NULL;

				switch (type) {
					case TINY:	{ iheap = heap_allocate(arena, TINY_SIZE, TINY);	break; }
					case SMALL:	{ iheap = heap_allocate(arena, SMALL_SIZE, SMALL);	break; }
					case LARGE:	{
						size_t total_size = (size + sizeof(t_chunk) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
						iheap = heap_allocate(arena, total_size, LARGE);
						if (iheap) {
							arena->alloc_count++;
							return ((void *)((char *)iheap->ptr + sizeof(t_chunk)));
						}
						break;
					}
				}

				// static int p = 0;
				// aprintf(1, "%p heap %d - %d\n", arena, p++, size);

				return (iheap);
			}
		
		#pragma endregion

	#pragma endregion

	#pragma region "Destroy"

		#pragma region "Free"

			// int heap_free(void *ptr, size_t size, e_heaptype type, t_heap *heap) {
			// 	if (!ptr || !size || type < 0 || type > 2 || !heap) {
			// 		if (g_manager.options.DEBUG && type != LARGE)		aprintf(2, "\t\t  [ERROR] Failed to detroy heap\n");
			// 		return (1);
			// 	}

			// 	t_heap *curr = heap;
			// 	while (curr) {
			// 		if (curr->active && curr->ptr == ptr) break;
			// 		curr = curr->next;
			// 	}

			// 	if (!curr) {
			// 		if (g_manager.options.DEBUG && type != LARGE)		aprintf(2, "\t\t  [ERROR] Failed to detroy heap\n");
			// 		return (1);
			// 	}

			// 	int result = 0;
			// 	if (munmap(ptr, size)) {
			// 		result = 1;
			// 		if (g_manager.options.DEBUG && type != LARGE)		aprintf(2, "\t\t  [ERROR] Failed to detroy heap\n");
			// 	}

			// 	curr->active = false;

			// 	if (!result && g_manager.options.DEBUG && type != LARGE) aprintf(2, "%p\t [SYSTEM] Heap of size (%d) freed\n", ptr, size);

			// 	return (result);
			// }

		#pragma endregion

		#pragma region "Destroy"

			// int heap_destroy(void *ptr, size_t size, e_heaptype type) {
			// 	if (!tcache || !ptr || !size || type < 0 || type > 2) return (1);

			// 	t_arena *arena = tcache;
			// 	int		result = 0;

			// 	switch (type) {
			// 		case TINY:	{ result = heap_free(ptr, size, TINY, arena->tiny);		break; }
			// 		case SMALL:	{ result = heap_free(ptr, size, SMALL, arena->small);	break; }
			// 		case LARGE:	{
			// 			result = heap_free(ptr, size, LARGE, arena->large);
			// 			if (!result) arena->free_count++;
			// 			break;
			// 		}
			// 	}

			// 	return (result);
			// }
		
		#pragma endregion

#pragma endregion

#pragma endregion
