CC = gcc
YACC = bison
LEX = flex

YACC_SRC = parser.y
LEX_SRC = lexer.l

YACC_C = parser.tab.c
YACC_H = parser.tab.h
LEX_C = lex.yy.c

OBJS = parser.tab.o lex.yy.o ast.o table.o ht_from_ast.o compiler.o
EXEC = compiler

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS) -lfl

$(YACC_C) $(YACC_H): $(YACC_SRC)
	$(YACC) -d $(YACC_SRC)

$(LEX_C): $(LEX_SRC) $(YACC_H)
	$(LEX) $(LEX_SRC)

%.o: %.c
	$(CC) -c $<

clean:
	rm -f $(EXEC) $(OBJS) $(YACC_C) $(YACC_H) $(LEX_C)

.PHONY: all clean
