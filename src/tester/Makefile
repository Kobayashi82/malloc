# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/20 10:30:48 by vzurera-          #+#    #+#              #
#    Updated: 2025/05/20 00:27:23 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Colores
GREEN   = \033[0;32m
YELLOW  = \033[0;33m
CYAN    = \033[0;36m
NC      = \033[0m

# Compilador y flags
CC		= gcc
CFLAGS	= -Wall -Wextra -Werror -g

# Rutas
MAIN_DIR = .
LIB_DIR  = ../../build/lib
INC_DIR	 = ../../inc

# Nombre del ejecutable
NAME	= test_malloc

# Archivos
SRCS	= main.c
OBJS	= $(SRCS:.c=.o)

# Librería de malloc
MALLOC_LIB = -L$(LIB_DIR) -lft_malloc

all: $(NAME)
	@printf "$(GREEN)✓ $(YELLOW)$(NAME)$(CYAN) compilado correctamente$(NC)\n"

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^ $(MALLOC_LIB) -pthread
	@printf "$(GREEN)Enlazando ejecutable $(NAME)$(NC)\n"

%.o: %.c
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@
	@printf "$(CYAN)Compilando $<...$(NC)\n"

clean:
	@rm -f $(OBJS)
	@printf "$(CYAN)Objetos eliminados$(NC)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "$(CYAN)Ejecutable eliminado$(NC)\n"

re: fclean all

run: $(NAME)
	@printf "$(GREEN)Ejecutando $(NAME) con la librería personalizada...$(NC)\n"
	@LD_PRELOAD=$(LIB_DIR)/libft_malloc.so ./$(NAME)

.PHONY: all clean fclean re run
