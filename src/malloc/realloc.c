/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 11:32:56 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/24 00:07:26 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

__attribute__((visibility("default")))
void *calloc(size_t nmemb, size_t size)
{
    size_t total;
    void *ptr;

    // Si alguno es 0, delegar a malloc(0)
    if (nmemb == 0 || size == 0)
        return (malloc(0));
    
    // Verificar overflow en la multiplicación
    if (nmemb > SIZE_MAX / size)
        return (NULL);
    
    total = nmemb * size;
    ptr = malloc(total);
    if (ptr)
        ft_memset(ptr, 0, total);
    
    return (ptr);
}

#pragma region "Realloc"

	__attribute__((visibility("default")))
	void *realloc(void *ptr, size_t size) {
		ensure_init();
		t_arena	*arena;
		void	*new_ptr = NULL;

		if (g_manager.options.DEBUG) aprintf(1, "\t\t[REALLOC] Solicitando %d bytes\n", size);

		if (!ptr)	return malloc(size);				// ptr NULL equivale a malloc(size)
		if (!size)	return (free(ptr), NULL);			// size es 0 equivale a free(ptr)

		if (!tcache) {
			arena = arena_get();
			tcache = arena;
			if (!arena) {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] Failed to assign arena\n");
				return (NULL);
			}
		} else arena = tcache;

		// En la implementación real:
		// 1. Verificar si el chunk actual puede ser extendido
		// 2. Buscar en bins
		// 3. Determinar zona
		// 4. Crear asignacion en el top chunk
		// 5. Liberar antiguo chunk si es necesario (bins)

		new_ptr = malloc(size);
		if (!new_ptr) return (NULL);


		// Trying something
		
		t_chunk *chunk = GET_HEAD(ptr);
		size_t copy_size = GET_SIZE(chunk); 							// must be ptr size
		if (size < copy_size) copy_size = size;
		ft_memcpy(new_ptr, ptr, copy_size);
		free(ptr);
		
		if (g_manager.options.DEBUG) aprintf(1, "%p\t[REALLOC] Memory reassigned from %p with %d bytes\n", new_ptr, ptr, size);

		return (new_ptr);
	}

#pragma endregion
