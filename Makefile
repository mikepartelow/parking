.PHONY: all clean nice run valgrind

NAME := parking
CC   ?= cc
CFLAGS ?= -Wall -Wextra
CFLAGS += -g -O0 -fno-omit-frame-pointer

SRC := $(NAME).c
OBJ := $(SRC:.c=.o)

FORMAT := clang-format
TIDY := clang-tidy
CPPCHECK := cppcheck

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(NAME) $(OBJ)

dbg: all
	lldb ./$(NAME)

nice:
	$(FORMAT) -i $(SRC)
	$(TIDY) $(SRC) -- -I. -std=c11;
	$(CPPCHECK) --enable=warning,style,performance,portability \
		--std=c11 --inline-suppr --error-exitcode=1 --check-level=exhaustive .

run: all
	./$(NAME)

valgrind:
	@docker build -f ./Dockerfile -t parking-valgrind .
	@docker rmi parking-valgrind
