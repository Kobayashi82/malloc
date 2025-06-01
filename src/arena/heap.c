/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 21:16:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"
	#include "utils.h"

#pragma endregion

#pragma region "PageSize"

	size_t get_pagesize() {
		#ifdef _WIN32
			SYSTEM_INFO info;
			GetSystemInfo(&info);
			return (info.dwPageSize);
		#else
			return ((size_t)sysconf(_SC_PAGESIZE));
		#endif
	}

#pragma endregion

#pragma region "Heap"

	#pragma region "Find"

		t_heap *heap_find(void *ptr, t_arena *arena) {
			if (!ptr || !arena) return (NULL);

			t_heap *curr = arena->tiny;
			while (curr) {
				if (ptr >= curr->ptr && ptr <= (void *)((char *)curr->ptr + curr->size)) return (curr);
				curr = curr->next;
			}
			curr = arena->small;
			while (curr) {
				if (ptr >= curr->ptr && ptr <= (void *)((char *)curr->ptr + curr->size)) return (curr);
				curr = curr->next;
			}
			curr = arena->large;
			while (curr) {
				if (ptr >= curr->ptr && ptr <= (void *)((char *)curr->ptr + curr->size)) return (curr);
				curr = curr->next;
			}

			return (NULL);
		}

	#pragma endregion

	#pragma region "Create"

		#pragma region "Allocate"

			static void *heap_allocate(size_t size, e_heaptype type, t_heap **heap) {
				if (!size || type < 0 || type > 2 || !heap) {
					if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to allocate %d bytes\n", size);
					return (NULL);
				}

				void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
				if (ptr == MAP_FAILED) {
					if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to allocate %d bytes\n", size);
					return (NULL);
				}

				t_heap *new_heap = internal_alloc(sizeof(t_heap));
				if (!new_heap) {
					if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to allocate heap structure\n");
					if (munmap(ptr, size) && g_manager.options.DEBUG) aprintf(1, "%p\t  [ERROR] Unable to unmap memory\n", ptr);
					return (NULL);
				}

				new_heap->ptr = ptr;
				new_heap->size = size;
				new_heap->free = size - ((type == LARGE) ? sizeof(t_lchunk) : sizeof(t_chunk));
				new_heap->type = type;
				new_heap->prev = NULL;
				new_heap->next = NULL;
				if ((*heap)) {
					new_heap->next = (*heap);
					(*heap)->prev = new_heap;
					(*heap) = new_heap;
				} else (*heap) = new_heap;

				if (type == LARGE) {
					t_lchunk *chunk = (t_lchunk *)ptr;
					chunk->prev_size = 0;
					chunk->size = (size - sizeof(t_lchunk)) | PREV_INUSE | (type == LARGE ? IS_MMAPPED : 0) | TOP_CHUNK;
					ptr = (void *)((char *)chunk + sizeof(t_lchunk));
				} else {
					t_chunk *chunk = (t_chunk *)ptr;
					chunk->prev_size = 0;
					chunk->size = (size - sizeof(t_chunk)) | PREV_INUSE | (type == LARGE ? IS_MMAPPED : 0) | TOP_CHUNK;
					ptr = (void *)((char *)chunk + sizeof(t_chunk));
				}

				if (g_manager.options.DEBUG) aprintf(1, "%p\t [SYSTEM] Heap of size (%d) allocated\n", new_heap->ptr, new_heap->size);
				return (ptr);
			}

		#pragma endregion

		#pragma region "Create"

			void *heap_create(e_heaptype type, size_t size) {
				if (!tcache || !size || type < 0 || type > 2) return (NULL);

				t_arena *arena = tcache;
				void	*ptr = NULL;

				mutex(&arena->mutex, MTX_LOCK);

					switch (type) {
						case TINY:	{
							ptr = heap_allocate(TINY_SIZE, TINY, &arena->tiny);
							break;
						}
						case SMALL:	{
							ptr = heap_allocate(SMALL_SIZE, SMALL, &arena->small);
							break;
						}
						case LARGE:	{
							size_t total_size = (size + sizeof(t_lchunk) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
							ptr = heap_allocate(total_size, LARGE, &arena->large);
							break;
						}
					}

				mutex(&arena->mutex, MTX_UNLOCK);

				return (ptr);
			}
		
		#pragma endregion

	#pragma endregion

	#pragma region "Destroy"

		#pragma region "Free"

			int heap_free(void *ptr, size_t size, e_heaptype type, t_heap **heap) {
				if (!ptr || !size || type < 0 || type > 2 || !heap || !*heap) {
					if		(g_manager.options.DEBUG && type != LARGE)	aprintf(1, "\t\t  [ERROR] Failed to detroy heap\n");
					else if (g_manager.options.DEBUG)					aprintf(1, "%p\t   [FREE] 1Failed to free memory\n", ptr);
					return (1);
				}

				t_heap *curr = *heap;
				while (curr) {
					if (curr->ptr == ptr) break;
					curr = curr->next;
				}

				if (!curr) {
					if		(g_manager.options.DEBUG && type != LARGE)	aprintf(1, "\t\t  [ERROR] Failed to detroy heap\n");
					else if (g_manager.options.DEBUG)					aprintf(1, "%p\t   [FREE] 2Failed to free memory\n", ptr);
					return (1);
				}

				int result = 0;
				if (munmap(ptr, size)) {
					result = 1;
					if		(g_manager.options.DEBUG && type != LARGE)	aprintf(1, "\t\t  [ERROR] Failed to detroy heap\n");
					else if (g_manager.options.DEBUG)					aprintf(1, "%p\t   [FREE] 3Failed to free memory\n", ptr);
					if		(g_manager.options.DEBUG)					aprintf(1, "%p\t  [ERROR] Unable to unmap memory\n", ptr);	
				}

				if (curr->prev) curr->prev->next = curr->next;
				if (curr->next) curr->next->prev = curr->prev;
				if (curr == *heap) *heap = curr->next;
				if (internal_free(curr, sizeof(t_heap))) {
					result = 1;
					if (g_manager.options.DEBUG)						aprintf(1, "\t\t  [ERROR] Failed to unmap heap structure\n");
				}

				if (!result) {
					if		(g_manager.options.DEBUG && type != LARGE)	aprintf(1, "%p\t [SYSTEM] Heap of size (%d) allocated\n", ptr);
					else if (g_manager.options.DEBUG)					aprintf(1, "%p\t   [FREE] Memory freed\n", ptr);
				}

				return (result);
			}

		#pragma endregion

		#pragma region "Destroy"

			int heap_destroy(void *ptr, size_t size, e_heaptype type) {
				if (!tcache || !ptr || !size || type < 0 || type > 2) return (1);

				t_arena *arena = tcache;
				int		result = 0;

				mutex(&arena->mutex, MTX_LOCK);

					switch (type) {
						case TINY:	{ result = heap_free(ptr, size, TINY, &arena->tiny);	break; }
						case SMALL:	{ result = heap_free(ptr, size, SMALL, &arena->small);	break; }
						case LARGE:	{ result = heap_free(ptr, size, LARGE, &arena->large);	break; }
					}

				mutex(&arena->mutex, MTX_UNLOCK);

				return (result);
			}
		
		#pragma endregion

#pragma endregion

#pragma endregion
