# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/30 11:13:36 by vzurera-          #+#    #+#              #
#    Updated: 2025/04/30 11:14:40 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ───────────────────────────────────────────────────────────── #
# ─────────────────────── CONFIGURATION ─────────────────────── #
# ───────────────────────────────────────────────────────────── #

# ──────────── #
# ── COLORS ── #
# ──────────── #

NC    				= \033[0m
RED     			= \033[0;31m
GREEN   			= \033[0;32m
YELLOW  			= \033[0;33m
BLUE    			= \033[0;34m
MAGENTA 			= \033[0;35m
CYAN    			= \033[0;36m
WHITE   			= \033[0;37m
INV_RED  			= \033[7;31m
INV_GREEN	  		= \033[7;32m
INV_YELLOW  		= \033[7;33m
INV_BLUE  			= \033[7;34m
INV_MAGENTA			= \033[7;35m
INV_CYAN			= \033[7;36m
INV_WHITE			= \033[7;37m
BG_CYAN				= \033[40m
FG_YELLOW			= \033[89m
COUNTER 			= 0

# ─────────── #
# ── FLAGS ── #
# ─────────── #

CC					= clang
# FLAGS				= -Wall -Wextra -Werror -O2							# (for production)
FLAGS				= -Wall -Wextra -Werror -g -O0 -gdwarf-4			# (for debugging)
# FLAGS				= -Wall -Wextra -Werror -g -O0 -fsanitize=thread	# (for debugging with sanitize)
EXTRA_FLAGS			= -ltermcap
EXTRA_FLAGS_OBJ		=

# ───────────────── #
# ── DIRECTORIES ── #
# ───────────────── #

INC_DIR				= $(shell find inc -type d | sed 's/^/-I/')
OBJ_DIR				= build/obj/
LIB_DIR				= build/lib/
SRC_DIR				= src/$(NAME)/

# ────────── #
# ── NAME ── #
# ────────── #

NAME	=	ft_malloc

SRCS	=	builtins/alias/alias.c						\
			builtins/alias/unalias.c					\
			builtins/env/declare.c						\
			builtins/env/export.c						\
			builtins/env/readonly.c						\
			builtins/env/set.c							\
			builtins/env/shopt.c						\
			builtins/env/unset.c						\
			builtins/filesystem/cd.c					\
			builtins/filesystem/hash.c					\
			builtins/filesystem/pwd.c					\
			builtins/history/fc.c						\
			builtins/history/history.c					\
			builtins/jobs/bg.c							\
			builtins/jobs/fg.c							\
			builtins/jobs/jobs.c						\
			builtins/jobs/kill.c						\
			builtins/jobs/wait.c						\
			builtins/math/let.c							\
			builtins/scripting/dot.c					\
			builtins/scripting/eval.c					\
			builtins/scripting/exec.c					\
			builtins/scripting/source.c					\
			builtins/scripting/test.c					\
			builtins/shell/banner.c						\
			builtins/shell/builtin.c					\
			builtins/shell/command.c					\
			builtins/shell/echo.c						\
			builtins/shell/enable.c						\
			builtins/shell/exit.c						\
			builtins/shell/help.c						\
			builtins/shell/type.c						\
			builtins/utils/executer.c					\
			builtins/utils/options.c					\
														\
														\
			hashes/alias.c								\
			hashes/builtin.c							\
			hashes/cmdp.c								\
			hashes/key_value.c							\
			hashes/variables.c							\
														\
														\
			main/error.c								\
			main/main.c									\
			main/options.c								\
			main/shell.c								\
														\
														\
			parser/expansions/alias/alias.c				\
			parser/expansions/arithmetic/arithmetic.c	\
			parser/expansions/brace/brace.c				\
			parser/expansions/globbing/brackets.c		\
			parser/expansions/globbing/globbing.c		\
			parser/expansions/globbing/match.c			\
			parser/expansions/globbing/pattern.c		\
			parser/expansions/history/history.c			\
			parser/expansions/parameter/parameter.c		\
			parser/expansions/shell/shell.c				\
														\
			parser/syntax/check.c						\
			parser/syntax/context.c						\
			parser/syntax/error.c						\
														\
			parser/tokenizer/args.c						\
			parser/tokenizer/lexer.c					\
			parser/tokenizer/parser.c					\
			parser/tokenizer/tokenizer.c				\
														\
														\
			terminal/readinput/common/auto_complete.c	\
			terminal/readinput/common/history_search.c	\
			terminal/readinput/common/history.c			\
			terminal/readinput/common/prompt.c			\
			terminal/readinput/common/termcaps.c		\
			terminal/readinput/common/undo.c			\
			terminal/readinput/modes/dumb.c				\
			terminal/readinput/modes/readline.c			\
			terminal/readinput/modes/vi.c				\
			terminal/readinput/readinput.c				\
														\
			terminal/input.c							\
			terminal/print.c							\
			terminal/signal.c							\
														\
														\
			utils/paths.c								\
			utils/times.c								\
														\
														\
			tests/builtin.c								\
			tests/create_args.c							\
			tests/internal.c							\
			tests/tests.c								\
			tests/untests.c								\

SRCS      := $(addprefix $(SRC_DIR), $(SRCS))

# ────────────────────────────────────────────────────────── #
# ───────────────────────── NORMAL ───────────────────────── #
# ────────────────────────────────────────────────────────── #

TARGET=$(if $(SRCS),$(NAME),empty)
TARGET=$(if $(NAME),$(NAME),empty)
all: $(TARGET)

empty:
#	Check if NAME is empty
	@rm -f .is_re; if [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ]; then printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; exit 1; fi

#all: $(NAME)

OBJS	= $(SRCS:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)
DEPS	= $(OBJS:.o=.d)
-include $(DEPS)

$(NAME): normal_extra $(OBJS)
#	Compile program
	@if [ -f $(NAME) ]; then \
		printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)"; \
	else \
		printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(NAME)$(NC)"; \
	fi
	@$(CC) $(FLAGS) $(INC_DIR) $(OBJS) $(LIB_DIR)$(LIBFT) $(EXTRA_FLAGS) -o $(NAME)
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"
#	Progress line
	@$(MAKE) -s _progress
#	----- REMOVE TO DISABLED TEST -----
	@if [ "$(TESTING)" -ne 0 ]; then \
		cp 42sh tester/42sh; \
		printf "\033[A" && ./leaks test || true; \
	fi
#	----- REMOVE TO DISABLED TEST -----
	@printf "\n"
#	Restore cursor
	@$(MAKE) -s _show_cursor

# ───────────── #
# ── OBJECTS ── #
# ───────────── #

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
#	Create folder
	@mkdir -p $(@D)
#	Compile objects
	@filename=$$(basename $<); filename=$${filename%.*}; \
	BAR=$$(printf "/ — \\ |" | cut -d" " -f$$(($(COUNTER) % 4 + 1))); \
	printf "\r%50s\r\t$(CYAN)Compiling... $(GREEN)$$BAR  $(YELLOW)$$filename$(NC)"; \
	$(eval COUNTER=$(shell echo $$(($(COUNTER)+1))))
	@$(CC) $(FLAGS) $(INC_DIR) -I$(LIBFT_INC) $(EXTRA_FLAGS_OBJ) -MMD -o $@ -c $<

# ───────────────── #
# ── EXTRA RULES ── #
# ───────────────── #

normal_extra:
#	Check if NAME is empty and source directory exists
	@if [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; rm -f .is_re; exit 1; fi
#	Hide cursor
	@$(MAKE) -s _hide_cursor
#	Create folders
	@mkdir -p build/lib
#	Title
	@if [ ! -f .is_re ]; then clear; $(MAKE) -s _title; fi; rm -f .is_re
#	Compile LIBFT
	@if [ -d "$(LIBFT_DIR)" ]; then \
		(make -s all -C $(LIBFT_DIR)/; exit 0); $(MAKE) -s _hide_cursor; \
	else \
		printf "\t$(WHITE)────────────────────────\n$(NC)"; \
	fi; printf "\n\t────────────────────────$(NC)\033[1A\r"

# ───────────────────────────────────────────────────────────── #
# ────────────────────────── RE-MAKE ────────────────────────── #
# ───────────────────────────────────────────────────────────── #

re:
#	Check if NAME is empty and source directory exists
	@rm -f .is_re; if [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; exit 1; fi
#	Hide cursor
	@$(MAKE) -s _hide_cursor
#	Title
	@clear; $(MAKE) -s _title
#	Delete LIBFT
	@if [ -d "$(LIBFT_DIR)" ]; then \
		(make -s -C $(LIBFT_DIR) fclean; exit 0); \
	else \
		printf "\t$(WHITE)────────────────────────\n$(NC)"; \
	fi
#	Delete objects
	@$(MAKE) -s _delete_objects
#	Delete $(NAME)
	@if [ -f $(NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(NAME)$(NC)"; \
		rm -f $(NAME); \
	fi
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"
	@$(MAKE) -s _progress; printf "\n"
#	Delete folder and files
	@-find build/$(LIB_DIR) -type d -empty -delete >/dev/null 2>&1 || true
	@-find build -type d -empty -delete >/dev/null 2>&1 || true
#	Create files
	@touch .is_re; printf "\033[1A\033[1A\r"
#	Execute $(NAME)
	@$(MAKE) -s $(NAME)

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── CLEAN ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

clean:
#	Check if NAME is empty
	@rm -f .is_re; if [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ]; then printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; exit 1; fi
#	Hide cursor
	@$(MAKE) -s _hide_cursor
#	Title
	@clear; $(MAKE) -s _title
#	Delete objects
	@if [ -d "$(LIBFT_DIR)" ]; then \
		(make -s _delete_objects -C $(LIBFT_DIR)/; exit 0); \
	else \
		printf "\t$(WHITE)────────────────────────\n$(NC)"; \
	fi
#	Delete objects
	@$(MAKE) -s _delete_objects 
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)objects$(NC)\n"
#	Progress line
	@$(MAKE) -s _progress; printf "\n"
#	Restore cursor
	@$(MAKE) -s _show_cursor

# ───────────────────────────────────────────────────────────── #
# ────────────────────────── F-CLEAN ────────────────────────── #
# ───────────────────────────────────────────────────────────── #

fclean:
#	Check if NAME is empty
	@rm -f .is_re; if [ ! -n "$(NAME)" ] || [ ! -n "$(SRCS)" ]; then printf "\n\t$(CYAN)source files doesn't exist\n\n$(NC)"; exit 1; fi
#	Hide cursor
	@$(MAKE) -s _hide_cursor
#	Title
	@clear; $(MAKE) -s _title
#	Delete LIBFT
	@if [ -d "$(LIBFT_DIR)" ]; then \
		(make -s -C $(LIBFT_DIR) fclean; exit 0); \
	else \
		printf "\t$(WHITE)────────────────────────\n$(NC)"; \
	fi
#	Delete objects
	@$(MAKE) -s _delete_objects
#	Delete $(NAME)
	@if [ -f $(NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)$(NAME)$(NC)"; \
		rm -f $(NAME); \
	fi
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"
	@$(MAKE) -s _progress; printf "\n"
#	Delete folder and files
	@-find build/$(LIB_DIR) -type d -empty -delete >/dev/null 2>&1 || true
	@-find build -type d -empty -delete >/dev/null 2>&1 || true
#	Restore cursor
	@$(MAKE) -s _show_cursor

# ───────────────────────────────────────────────────────────── #
# ───────────────────────── FUNCTIONS ───────────────────────── #
# ───────────────────────────────────────────────────────────── #

# ─────────── #
# ── TITLE ── #
# ─────────── #

_title:
	@printf "\n$(NC)\t$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★\
	$(INV_CYAN)    $(NC)$(INV_CYAN)$(shell echo $(NAME) | tr a-z A-Z | tr '_' ' ')$(INV_CYAN)    \
	$(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(NC)\n"

# ───────────── #
# ── CURSORS ── #
# ───────────── #

_hide_cursor:
	@printf "\e[?25l"
 
_show_cursor:
	@printf "\e[?25h"

# ──────────────────── #
# ── DELETE OBJECTS ── #
# ──────────────────── #

_delete_objects:
	@printf "\n\t$(WHITE)────────────────────────$(NC)\033[1A\r"
	@if [ -n "$(shell find $(OBJ_DIR) -type f -name '*.o' 2>/dev/null)" ]; then \
		COUNTER=0; \
		find $(OBJ_DIR) -type f -name '*.o' | while read -r file; do \
			BAR=$$(printf "/ — \\ |" | cut -d" " -f$$((COUNTER % 4 + 1))); \
			filename=$$(basename $$file); \
			srcpath=$$(find $(SRC_DIR) -type f -name "$${filename%.o}.*" 2>/dev/null); \
			if [ -n "$$srcpath" ]; then \
				rm -f $$file $$(dirname $$file)/$${filename%.o}.d; \
				filename=$${filename%.*}; \
				printf "\r%50s\r\t$(CYAN)Deleting... $(GREEN)$$BAR $(YELLOW)$$filename$(NC)"; sleep 0.05; \
				COUNTER=$$((COUNTER+1)); \
			fi; \
		done; \
	fi; printf "\r%50s\r"
	@-find $(OBJ_DIR) -type d -empty -delete >/dev/null 2>&1 || true

wipe:
	@rm -rf build ; rm -f .is_re $(NAME)

# ─────────────────── #
# ── PROGRESS LINE ── #
# ─────────────────── #

_progress:
	@total=24; printf "\r\t"; for i in $$(seq 1 $$total); do printf "$(RED)─"; sleep 0.01; done; printf "$(NC)"
	@total=24; printf "\r\t"; for i in $$(seq 1 $$total); do printf "─"; sleep 0.01; done; printf "\n$(NC)"

# ─────────── #
# ── PHONY ── #
# ─────────── #

.PHONY: all clean fclean re normal_extra wipe _delete_objects _title _hide_cursor _show_cursor _progress