.PHONY: all clean

NAME := parking
CC   ?= cc
CFLAGS ?= -Wall -Wextra
CFLAGS += -g -O0 -fno-omit-frame-pointer

SRC := $(NAME).c
OBJ := $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(NAME) $(OBJ)
