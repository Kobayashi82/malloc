/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bin.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 09:10:09 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/28 14:02:01 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

	#include <stdint.h>

typedef struct s_bin t_bin;
typedef struct s_bin {
	void	*ptr;							// Pointer to the free block
	t_bin	*next;							// Pointer to next free block

} t_bin;

	void	set_bit(uint64_t *bitmap, int pos);
	void	clear_bit(uint64_t *bitmap, int pos);
	int		is_bit_set(uint64_t *bitmap, int pos);
	int		find_first_free_bit(uint64_t *bitmap, int max_bits);
