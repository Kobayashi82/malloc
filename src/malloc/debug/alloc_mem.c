/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   alloc_mem.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:15:02 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/01 12:20:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Load"

	#pragma region "Count Heaps"

		static void count_heaps(t_heap_header *heap_header, int *heaps_count, int *tiny_count, int *small_count, int *large_count) {
			if (!heap_header) return ;

			while (heap_header) {
				t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

				for (int i = 0; i < heap_header->used; ++i) {
					if (heap->active) {
						(*heaps_count)++;
						if (heap->type == TINY) (*tiny_count)++;
						if (heap->type == SMALL) (*small_count)++;
						if (heap->type == LARGE) (*large_count)++;
					}
					heap = (t_heap *)((char *)heap + ALIGN(sizeof(t_heap)));
				}

				heap_header = heap_header->next;
			}
		}

	#pragma endregion

	#pragma region "Load Heaps"

		static void load_heaps(t_heap_header *heap_header, t_heap **heaps, int heaps_count) {
			if (!heap_header || !heaps) return ;

			int index = 0;

			while (heap_header) {
				t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

				for (int i = 0; i < heap_header->used; ++i) {
					if (heap->active) {
						heaps[index] = heap;
						index++;
					}

					if (index == heaps_count) {
						heaps[index] = NULL;
						return ;
					}
					heap = (t_heap *)((char *)heap + ALIGN(sizeof(t_heap)));
				}

				heap_header = heap_header->next;
			}
		}

	#pragma endregion

	#pragma region "Sort Heaps"

		static void sort_heaps(t_heap **heaps) {
			if (!heaps || !heaps[0]) return;

			for (size_t i = 0; heaps[i + 1]; i++) {
				for (size_t j = 0; heaps[j + 1]; j++) {
					if (heaps[j]->ptr > heaps[j + 1]->ptr) {
						t_heap *temp = heaps[j];
						heaps[j] = heaps[j + 1];
						heaps[j + 1] = temp;
					}
				}
			}
		}

	#pragma endregion

#pragma endregion

#pragma region "Print"

	#pragma region "Print HEX8"

		static void print_hex8(int fd, uintptr_t v) {
			const char *hex = "0123456789ABCDEF";
			char buf[10];

			buf[0] = '0';
			buf[1] = 'x';
			for (int i = 0; i < 8; ++i)
				buf[2 + i] = hex[(v >> ((7 - i) * 4)) & 0xF];
			write(fd, buf, 10);
		}

	#pragma endregion

	#pragma region "Print Heap"

		static size_t print_heap(t_heap *heap) {
			if (!heap) return (0);

			t_chunk	*chunk = heap->ptr;
			size_t	total = 0;

			while (chunk) {
				if (IS_TOPCHUNK(chunk) && heap->type != LARGE) break;
				if (heap->type == LARGE || !IS_FREE(chunk)) {
					write(2, " ", 1);
					print_hex8(1,  (uintptr_t)GET_PTR(chunk));
					write(2, " - ", 3);
					if (heap->type == LARGE)	print_hex8(1,  (uintptr_t)(char*)GET_PTR(chunk) + GET_SIZE(chunk) - 1);
					else						print_hex8(1,  (uintptr_t)((char *)GET_NEXT(chunk) - 1));
					aprintf(2, 0, " : %u bytes\n", GET_SIZE(chunk));
					total += GET_SIZE(chunk);
					if (heap->type == LARGE) break;
				}
				chunk = GET_NEXT(chunk);
			}

			return (total);
		}

	#pragma endregion

	#pragma region "Print Heaps"

		static size_t print_heaps(t_arena *arena, t_heap **heaps, int heaps_count, int tiny_count, int small_count, int large_count) {
			if (!arena || !heaps) return (0);

			aprintf(2, 0, "————————————\n");
			aprintf(2, 0, " • Arena #%d\n", arena->id);
			aprintf(2, 0, "—————————————————————————————————————————\n");
			aprintf(2, 0, " • Allocations: %u\t• Frees: %u\n", arena->alloc_count, arena->free_count);
			aprintf(2, 0, " • TINY: %u \t\t• SMALL: %u\n", tiny_count, small_count);
			aprintf(2, 0, " • LARGE: %u\t\t• TOTAL: %u\n", large_count, heaps_count);
			aprintf(2, 0, "—————————————————————————————————————————\n\n");

			size_t arena_total = 0;

			for (size_t i = 0; heaps[i]; i++) {
				t_heap *heap = heaps[i];

				char *type;
				if (heaps[i]->type == TINY) type = "TINY";
				if (heaps[i]->type == SMALL) type = "SMALL";
				if (heaps[i]->type == LARGE) type = "LARGE";
				
				if (i > 0) aprintf(2, 0, "\n");

				aprintf(2, 0, " %s : ", type);
				print_hex8(1,  (uintptr_t)heap->ptr);
				write(2, "\n", 1);
				aprintf(2, 0, "— — — — — — — — — — — — — — — — — — — — —\n");
				size_t heap_total = print_heap(heaps[i]);
				arena_total += heap_total;

				if (heap_total) {
					aprintf(2, 0, "                          — — — — — — — —\n");
					aprintf(2, 0, "                           %u byte%s\n", heap_total, heap_total == 1 ? "" : "s");
				}
			}

			if (arena_total) {
				aprintf(2, 0, "\n——————————————————————————————————————————\n");
				aprintf(2, 0, " %u byte%s in arena #%d\n\n\n", arena_total, arena_total == 1 ? "" : "s", arena->id);
			}

			return (arena_total);
		}

	#pragma endregion

#pragma endregion

#pragma region "Show Alloc Mem"

	__attribute__((visibility("default")))
	void show_alloc_mem() {
	
		mutex(&g_manager.mutex, MTX_LOCK);

			t_arena *arena = &g_manager.arena;
			size_t	total = 0;
			int alloc_count = 0;
			int free_count = 0;

			for (int i = 0; i < g_manager.arena_count; ++i) {
				if (!arena) break;
				int heaps_count = 0;
				int tiny_count = 0;
				int small_count = 0;
				int large_count = 0;

				mutex(&arena->mutex, MTX_LOCK);

					alloc_count += arena->alloc_count;
					free_count += arena->free_count;
					count_heaps(arena->heap_header, &heaps_count, &tiny_count, &small_count, &large_count);
					if (heaps_count) {
						t_heap *heaps[heaps_count + 1];
						ft_memset(heaps, 0, sizeof(heaps));
						load_heaps(arena->heap_header, heaps, heaps_count);
						sort_heaps(heaps);
						total += print_heaps(arena, heaps, heaps_count, tiny_count, small_count, large_count);
					}

				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}

			
			if (!total) aprintf(2, 0, "No memory has been allocated\n");
			else if (g_manager.arena_count > 0) {
				aprintf(2, 0, "———————————————————————————————————————————————————————————————\n");
				aprintf(2, 0, " • %d allocation%s, %d free%s and %u byte%s across %d arena%s\n", alloc_count, alloc_count == 1 ? "" : "s", free_count, free_count == 1 ? "" : "s", total, total == 1 ? "" : "s", g_manager.arena_count, g_manager.arena_count == 1 ? "" : "s");
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);
	}

#pragma endregion

#pragma region "Information"

	// Shows a detailed map of all current allocations.
	//
	//   void show_alloc_mem(void);
	//
	//   • On success: prints a detailed report of all current allocations.
	//
	// Notes:
	//   • Output is written to file descriptor 1 (stdout).
	//   • Heaps are sorted before printing, and grouped by arena.

#pragma endregion
