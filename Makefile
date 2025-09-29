CC = gcc
BISON = bison
FLEX = flex

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

BISON_SRC = $(SRC_DIR)/parser.y
FLEX_SRC = $(SRC_DIR)/lexer.l

BISON_C = $(SRC_DIR)/parser.tab.c
BISON_H = $(INC_DIR)/parser.tab.h
FLEX_C = $(SRC_DIR)/lex.yy.c

SRCS = $(BISON_C) $(FLEX_C) \
			 $(SRC_DIR)/ast.c $(SRC_DIR)/table.c \
			 $(SRC_DIR)/label.c $(SRC_DIR)/ht_from_ast.c \
			 $(SRC_DIR)/emit.c $(SRC_DIR)/gen.c\
			 $(SRC_DIR)/gen_mixal_from_ast.c \
			 $(SRC_DIR)/main.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
EXEC = compiler

ASCII_FLAG = true
DEBUG_FLAG = false

CFLAGS = -I$(INC_DIR) -DASCII=$(ASCII_FLAG) -DDEBUG=$(DEBUG_FLAG) -ggdb
LDFLAGS = -lfl

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BISON_C) $(BISON_H): $(BISON_SRC)
	$(BISON) --header=$(BISON_H) --output=$(BISON_C) $(BISON_SRC)

$(FLEX_C): $(FLEX_SRC) $(BISON_H)
	$(FLEX) -o $(FLEX_C) $(FLEX_SRC)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(FLEX_C) $(BISON_C) $(BISON_H) $(EXEC)

.PHONY: all clean
