/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 22:11:24 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/31 21:40:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"
	#include "utils.h"

#pragma endregion

// Para obtener el tamaño real:

// size_t real_size = chunk->size & ~(IS_MMAPPED | PREV_INUSE);

// Para verificar si es mapeado:

// if (chunk->size & IS_MMAPPED) {
//     // Es un chunk mapeado, usar munmap()
// } else {
//     // Es chunk normal, manejar en arena
// }

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

#pragma region "Range"

	#pragma region "Delete"

		#pragma region "Del Array"

			int range_del_array(t_range *range, void *ptr, size_t size) {
				bool	shrink = false;
				int		result = 1;

				for (int i = 0; i < HEAPS_MAX && range->start[i] && range->end[i]; ++i) {
					if (!shrink && range->start[i] == ptr && range->end[i] == ptr + size) shrink = true;
					if (shrink) {
						result = 0;
						int move = HEAPS_MAX - i - 1;
						if (move > 0) {
							memmove(&range->start[i], &range->start[i + 1], move * sizeof(void *));
							memmove(&range->end[i], &range->end[i + 1], move * sizeof(void *));
						}
						range->start[HEAPS_MAX - 1] = NULL;
						range->end[HEAPS_MAX - 1] = NULL;
						break;
					}
				}

				return (result);
			}

		#pragma endregion

		#pragma region "Del Range"

			int range_del(void *ptr, size_t size) {
				if (!ptr || !tcache.arena) return (1);

				int		result = 0;
				t_arena	*arena = tcache.arena;

				mutex(&g_manager.mutex, MTX_LOCK);
				mutex(&arena->mutex, MTX_LOCK);

					result += range_del_array(&arena->range, ptr, size);
					result += range_del_array(&g_manager.range[arena->id], ptr, size);

				mutex(&arena->mutex, MTX_UNLOCK);
				mutex(&g_manager.mutex, MTX_UNLOCK);

				return (result != 0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Add"

		#pragma region "Add Array"

			int range_add_array(t_range *range, void *ptr, size_t size) {
				int i = 0;

				while (i < HEAPS_MAX && range->start[i] && range->end[i]) ++i;
				if (i == HEAPS_MAX) return (1);
				range->start[i] = ptr;
				range->end[i] = ptr + size;

				return (0);
			}

		#pragma endregion

		#pragma region "Add Range"

			int range_add(void *ptr, size_t size) {
				if (!ptr || !tcache.arena) return (1);

				int		result = 0;
				t_arena	*arena = tcache.arena;
				
				mutex(&g_manager.mutex, MTX_LOCK);
				mutex(&arena->mutex, MTX_LOCK);

					if (!(result = range_add_array(&arena->range, ptr, size))) {
						if ((result = range_add_array(&g_manager.range[arena->id], ptr, size)))
							range_del_array(&arena->range, ptr, size);
					}

				mutex(&arena->mutex, MTX_UNLOCK);
				mutex(&g_manager.mutex, MTX_UNLOCK);

				return (result);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "Heap"

	#pragma region "Create"

		void *heap_create(e_heaptype type, size_t size) {
			if (!tcache.arena || !size || type < 0 || type > 2) return (NULL);

			t_arena *arena = tcache.arena;
			void	*ptr = NULL;

			mutex(&arena->mutex, MTX_LOCK);

				switch (type) {
					case TINY:	{
						ptr = mmap(NULL, TINY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
						if (ptr == MAP_FAILED) ptr = NULL;

						if (!ptr) {
							t_chunk *chunk = (t_chunk *)ptr;
							chunk->prev_size = 0;
							chunk->size = (TINY_SIZE - sizeof(t_chunk)) | PREV_INUSE | TOP_CHUNK;
							ptr = (void *)((char *)chunk + sizeof(t_chunk));
							range_add(ptr, TINY_SIZE);
						} else if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to allocate %d bytes\n", size);
						break;
					}
					case SMALL:	{
						ptr = mmap(NULL, SMALL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
						if (ptr == MAP_FAILED) ptr = NULL;
						if (!ptr) {
							t_chunk *chunk = (t_chunk *)ptr;
							chunk->prev_size = 0;
							chunk->size = (SMALL_SIZE - sizeof(t_chunk)) | PREV_INUSE | TOP_CHUNK;
							ptr = (void *)((char *)chunk + sizeof(t_chunk));
							range_add(ptr, SMALL_SIZE);
						} else if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to allocate %d bytes\n", size);
						break;
					}
					case LARGE:	{
						size_t total_size = (size + sizeof(t_chunk) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
						ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
						if (ptr == MAP_FAILED) ptr = NULL;
						if (ptr) {
							t_chunk *chunk = (t_chunk *)ptr;
							chunk->prev_size = 0;
							chunk->size = total_size | PREV_INUSE | IS_MMAPPED | TOP_CHUNK;
							ptr = (void *)((char *)chunk + sizeof(t_chunk));
							range_add(ptr, total_size);
						} else if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to allocate %d bytes\n", size);
						break;
					}
				}

			mutex(&arena->mutex, MTX_UNLOCK);

			return (ptr);
		}
	
	#pragma endregion

#pragma endregion
