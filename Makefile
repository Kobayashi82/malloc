# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/18 11:22:48 by vzurera-          #+#    #+#              #
#    Updated: 2025/07/05 17:01:45 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

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

# ────────── #
# ── NAME ── #
# ────────── #

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
else ifeq ($(HOSTTYPE), $(shell uname -m))
	HOSTTYPE := $(HOSTTYPE)_$(shell uname -s)
endif

NAME		= malloc
LIB_NAME	= libft_$(NAME)_$(HOSTTYPE)$(LIB_EXT)

# ─────────── #
# ── FLAGS ── #
# ─────────── #

CC			= clang

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	FLAGS		= -Wall -Wextra -Werror -fPIC
	SHARED_FLAG	= -dynamiclib
	LIB_EXT		= .dylib
else
	FLAGS		= -Wall -Wextra -Werror -fPIC -fvisibility=hidden
	SHARED_FLAG	= -shared
	LIB_EXT		= .so
endif

# ───────────────── #
# ── DIRECTORIES ── #
# ───────────────── #

INC_DIR		= inc/
BLD_DIR		= build/
OBJ_DIR		= $(BLD_DIR)obj/
LIB_DIR		= lib/
SRC_DIR		= src/

# ─────────── #
# ── FILES ── #
# ─────────── #

SRCS		= internal/internal.c internal/options.c					\
\
			  arena/arena.c arena/heap.c arena/bin.c arena/allocation.c	\
\
			  malloc/main/free.c malloc/main/malloc.c					\
			  malloc/main/realloc.c malloc/main/calloc.c				\
\
			  malloc/extra/reallocarray.c malloc/extra/aligned_alloc.c	\
			  malloc/extra/memalign.c malloc/extra/posix_memalign.c		\
			  malloc/extra/valloc.c malloc/extra/pvalloc.c				\
			  malloc/extra/malloc_usable_size.c							\
\
			  malloc/debug/mallopt.c malloc/debug/alloc_hist.c			\
			  malloc/debug/alloc_mem.c malloc/debug/alloc_mem_ex.c		\
\
			  utils/string.c utils/number.c utils/mem.c utils/aprintf.c

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── RULES ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

all: _show_title $(LIB_DIR)$(LIB_NAME)
$(NAME): _show_title $(LIB_DIR)$(LIB_NAME)

SRC_PATHS	= $(addprefix $(SRC_DIR), $(SRCS))
OBJS		= $(SRCS:%.c=$(OBJ_DIR)%.o)
DEPS		= $(OBJS:.o=.d)
-include $(DEPS)

$(LIB_DIR)$(LIB_NAME): $(OBJS)
	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(LIB_DIR)

#	Compile library
	@printf "\r%50s\r\t$(CYAN)Compiling... $(YELLOW)$(NAME)$(NC)"
	@$(CC) $(SHARED_FLAG) -o $(LIB_DIR)$(LIB_NAME) $(OBJS)
	@printf "\r%50s\r\t$(CYAN)Compiled    $(GREEN)✓ $(YELLOW)$(NAME)$(NC)\n"

#   Symbolic link
	@cd $(LIB_DIR) && ln -sf $(LIB_NAME) libft_$(NAME)$(LIB_EXT)

	@$(MAKE) -s _progress; printf "\n"

	@if [ -f ./tester/load.sh ]; then chmod +x ./tester/load.sh; fi
	@if [ -f ./tester/unload.sh ]; then chmod +x ./tester/unload.sh; fi
	@printf " $(CYAN)To load the library run:\t$(YELLOW)source ./tester/load.sh$(NC)\n"
	@printf " $(CYAN)To evaluate the library run:\t$(YELLOW)source ./tester/eval.sh$(NC)\n\n"

	@$(MAKE) -s _show_cursor
	@rm -f /tmp/malloc_*.log 2> /dev/null

# ───────────── #
# ── OBJECTS ── #
# ───────────── #

-include $(DEPS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@$(MAKE) -s _hide_cursor
#	Create folder
	@mkdir -p $(@D)

#	Compile objects
	@filename=$$(basename $<); filename=$${filename%.*}; \
	BAR=$$(printf "/ — \\ |" | cut -d" " -f$$(($(COUNTER) % 4 + 1))); \
	printf "\r%50s\r\t$(CYAN)Compiling... $(GREEN)$$BAR $(YELLOW)$$filename$(NC)"; \
	$(eval COUNTER=$(shell echo $$(($(COUNTER)+1))))
	@$(CC) $(FLAGS) -I$(INC_DIR) -MMD -o $@ -c $<

# ───────────────── #
# ── EXTRA RULES ── #
# ───────────────── #

_show_title:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title

#	Check if source exists and needs recompiling
	@if  [ ! -n "$(LIB_NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	elif [ -f "$(LIB_DIR)$(LIB_NAME)" ] && \
		[ -z "$$(find $(SRC_PATHS) -newer "$(LIB_DIR)$(LIB_NAME)" 2>/dev/null; find inc -name '*.h' -newer "$(LIB_DIR)$(LIB_NAME)" 2>/dev/null)" ] && \
		[ $$(find $(OBJS) 2>/dev/null | wc -l) -eq $$(echo "$(OBJS)" | wc -w) ]; then \
        printf "\t$(GREEN)✓ $(YELLOW)malloc$(CYAN) is up to date$(NC)"; \
		printf "\n\t$(WHITE)────────────────────────$(NC)"; \
		$(MAKE) -s _progress; printf "\n" \
		$(MAKE) -s _show_cursor; \
	else \
		printf "\n\t$(WHITE)────────────────────────$(NC)\033[1A\r"; \
	fi
	@$(MAKE) -s _show_cursor

# ───────────────────────────────────────────────────────────── #
# ───────────────────────── RE - CLEAN ─────────────────────────#
# ───────────────────────────────────────────────────────────── #

# ───────────── #
# ── RE-MAKE ── #
# ───────────── #

re:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title

#	Check if source exists and needs recompiling
	@if  [ ! -n "$(LIB_NAME)" ] || [ ! -n "$(SRCS)" ] || [ ! -d "$(SRC_DIR)" ]; then \
        printf "\t$(CYAN)Source files don't exist$(NC)"; \
		printf "\n\t$(WHITE)────────────────────────$(NC)"; \
		$(MAKE) -s _progress; \
		$(MAKE) -s _show_cursor; \
	fi

#	Delete objects and library
	@$(MAKE) -s _delete_objects
	@if [ -f $(LIB_DIR)$(LIB_NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)library$(NC)"; \
		rm -f $(LIB_DIR)$(LIB_NAME); \
	fi
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)library$(NC)\n"
	@$(MAKE) -s _progress; printf "\n"
	@-find $(BLD_DIR) -type d -empty -delete >/dev/null 2>&1 || true
	@printf "\t$(WHITE)────────────────────────\n$(NC)"
	@printf "\033[1A\033[1A\r"

#	Compile library
	@$(MAKE) -s $(LIB_DIR)$(LIB_NAME)

# ─────────── #
# ── CLEAN ── #
# ─────────── #

clean:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title

	@$(MAKE) -s _delete_objects
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)objects$(NC)\n"

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor
	@rm -f /tmp/malloc_*.log 2> /dev/null

# ──────────── #
# ── FCLEAN ── #
# ──────────── #

fclean:
	@$(MAKE) -s _hide_cursor
	@$(MAKE) -s _title

	@$(MAKE) -s _delete_objects
	@if [ -f $(LIB_DIR)$(LIB_NAME) ]; then \
		printf "\t$(CYAN)Deleting... $(YELLOW)library$(NC)"; \
		rm -f $(LIB_DIR)$(LIB_NAME); \
		rm -f $(LIB_DIR)libft_malloc$(LIB_EXT); \
	fi
	@printf "\r%50s\r\t$(CYAN)Deleted     $(GREEN)✓ $(YELLOW)library$(NC)\n"
	@find $(BLD_DIR) -type d -empty -delete >/dev/null 2>&1 || true
	@find $(LIB_DIR) -type d -empty -delete >/dev/null 2>&1 || true

	@$(MAKE) -s _progress; printf "\n"
	@$(MAKE) -s _show_cursor
	@rm -f /tmp/malloc_*.log 2> /dev/null

# ───────────────────────────────────────────────────────────── #
# ───────────────────────── FUNCTIONS ───────────────────────── #
# ───────────────────────────────────────────────────────────── #

# ─────────── #
# ── TITLE ── #
# ─────────── #

_title:
	@clear
	@printf "\n$(NC)\t$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★\
	$(INV_CYAN)   $(NC)$(INV_CYAN)$(shell echo $(NAME) | tr a-z A-Z | tr '_' ' ')$(INV_CYAN)   \
	$(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(BG_CYAN)$(FG_YELLOW)★$(INV_CYAN) $(NC)\n"
	@printf "\t$(WHITE)────────────────────────\n$(NC)"

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
	@-find $(BLD_DIR) -type d -empty -delete >/dev/null 2>&1 || true

wipe:
	@rm -rf $(BLD_DIR)
	@rm -rf $(LIB_DIR)
	@rm -f /tmp/malloc_*.log 2> /dev/null

# ─────────────────── #
# ── PROGRESS LINE ── #
# ─────────────────── #

_progress:
	@total=24; printf "\r\t"; for i in $$(seq 1 $$total); do printf "$(RED)─"; sleep 0.01; done; printf "$(NC)"
	@total=24; printf "\r\t"; for i in $$(seq 1 $$total); do printf "─"; sleep 0.01; done; printf "\n$(NC)"

# ─────────── #
# ── PHONY ── #
# ─────────── #

.PHONY: all clean fclean re wipe _show_title _title _hide_cursor _show_cursor _delete_objects _progress
