/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mallopt.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/29 12:16:03 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 12:57:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Mallopt"

	__attribute__((visibility("default")))
	int mallopt(int param, int value) {
		return (options_set(param, value));
	}

#pragma endregion
