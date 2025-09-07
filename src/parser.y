%{
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

/* declare the lexer function */
int yylex(void);

/* declare the error handler */
void yyerror(ASTNode **ast, const char *s);
%}


%code requires {
  typedef struct ASTNode ASTNode;
  typedef struct ASTList ASTList;

  enum OpKind {
    OP_RELOP_LEQ, OP_RELOP_LT,
    OP_RELOP_GT, OP_RELOP_GEQ,
    OP_RELOP_EQ, OP_RELOP_NEQ,
    OP_ADDOP_ADD, OP_ADDOP_SUB,
    OP_MULOP_MUL, OP_MULOP_DIV
  };

  enum DataType {
    TYPE_INT
  };
}

%parse-param { ASTNode **ast }
%locations

%token IF ELSE WHILE RETURN BREAK
%token TYPE
%token TRUE FALSE
%token IDENTIFIER NUMBER
%token RELOP ADDOP MULOP

%union {
  ASTNode *node;
  ASTList *list;
  int num;
  char *id;
  enum DataType type;
  enum OpKind op;
}

%type <node> PROGRAM METH BODY DECL STMT BLOCK ASSIGN EXPR ADDEXPR TERM FACTOR 
%type <list> METHLIST PARAMS FORMALS DECLS DECLLIST VARS STMTS ACTUALS ARGS
%type <id> LOCATION METHOD

%type <num> NUMBER
%type <id> IDENTIFIER
%type <type> TYPE
%type <op> RELOP ADDOP MULOP

%%

PROGRAM:
      /* empty */ { *ast = ast_new_program(NULL, @$); }
    | METHLIST    { *ast = ast_new_program($1, @$); }
    ;

METHLIST:
      METH METHLIST { $$ = ast_list_prepend($2, $1); }
    | METH          { $$ = ast_list_prepend(NULL, $1); }
    ;

METH:
      TYPE IDENTIFIER '(' PARAMS ')' BODY {
        $$ = ast_new_method($1, $2, ast_list_reverse($4), $6, @$);
      }
    ;

PARAMS:
      /* empty */             { $$ = NULL; }
    | FORMALS TYPE IDENTIFIER {
        YYLTYPE loc;
        loc.first_line   = @2.first_line;
        loc.first_column = @2.first_column;
        loc.last_line    = @3.last_line;
        loc.last_column  = @3.last_column;
        ASTNode *p = ast_new_param($2, $3, loc);
        $$ = ast_list_prepend($1, p);
      }
    ;

FORMALS:
      /* empty */                 { $$ = NULL; }
    | FORMALS TYPE IDENTIFIER ',' {
        YYLTYPE loc;
        loc.first_line   = @2.first_line;
        loc.first_column = @2.first_column;
        loc.last_line    = @3.last_line;
        loc.last_column  = @3.last_column;
        ASTNode *p = ast_new_param($2, $3, loc);
        $$ = ast_list_prepend($1, p);
      }
    ;

BODY:
      '{' DECLS STMTS '}' {
        $$ = ast_new_body(ast_list_reverse($2), ast_list_reverse($3), @$);
      }
    ;

DECLS:
      /* empty */          { $$ = NULL; }
    | DECLLIST DECL        { $$ = ast_list_prepend($1, $2); }
    ;

DECLLIST:
      /* empty */          { $$ = NULL; }
    | DECLLIST DECL        { $$ = ast_list_prepend($1, $2); }
    ;

DECL:
      TYPE IDENTIFIER VARS ';'          {
        ASTNode *v = ast_new_var($2, NULL, @2);
        ASTList *vars = ast_list_prepend($3, v);
        $$ = ast_new_decl($1, vars, @$);
      }
    | TYPE IDENTIFIER '=' EXPR VARS ';' {
        YYLTYPE loc;
        loc.first_line   = @2.first_line;
        loc.first_column = @2.first_column;
        loc.last_line    = @4.last_line;
        loc.last_column  = @4.last_column;
        ASTNode *v = ast_new_var($2, $4, loc);
        ASTList *vars = ast_list_prepend($5, v);
        $$ = ast_new_decl($1, vars, @$);
      }
    ;

VARS:
      /* empty */                  { $$ = NULL; }
    | ',' IDENTIFIER VARS          {
        ASTNode *v = ast_new_var($2, NULL, @2);
        $$ = ast_list_prepend($3, v);
      }
    | ',' IDENTIFIER '=' EXPR VARS {
        YYLTYPE loc;
        loc.first_line   = @2.first_line;
        loc.first_column = @2.first_column;
        loc.last_line    = @4.last_line;
        loc.last_column  = @4.last_column;
        ASTNode *v = ast_new_var($2, $4, loc);
        $$ = ast_list_prepend($5, v);
      }
    ;

STMTS:
      /* empty */        { $$ = NULL; }
    | STMTS STMT         { $$ = ast_list_prepend($1, $2); }
    ;

STMT:
      ASSIGN ';'                       { $$ = $1; $$->loc = @$; }
    | RETURN EXPR ';'                  { $$ = ast_new_return($2, @$); }
    | IF '(' EXPR ')' STMT ELSE STMT   { $$ = ast_new_if($3, $5, $7, @$); }
    | WHILE '(' EXPR ')' STMT          { $$ = ast_new_while($3, $5, @$); }
    | BREAK ';'                        { $$ = ast_new_break(@$); }
    | BLOCK                            { $$ = $1; }
    | ';'                              { $$ = NULL; }
    ;

BLOCK:
      '{' STMTS '}' { $$ = ast_new_block(ast_list_reverse($2), @$); }
    ;

ASSIGN:
      LOCATION '=' EXPR { $$ = ast_new_assign($1, $3, @$); }
    ;

LOCATION:
      IDENTIFIER { $$ = $1; }
    ;

METHOD:
      IDENTIFIER { $$ = $1; }
    ;

EXPR:
      ADDEXPR RELOP ADDEXPR { $$ = ast_new_expr($2, $1, $3, @$); }
    | ADDEXPR               { $$ = $1; }
    ;

ADDEXPR:
      ADDEXPR ADDOP TERM { $$ = ast_new_expr($2, $1, $3, @$); }
    | TERM               { $$ = $1; }
    ;

TERM:
      TERM MULOP FACTOR { $$ = ast_new_expr($2, $1, $3, @$); }
    | FACTOR            { $$ = $1; }
    ;

FACTOR:
      '(' EXPR ')'           { $$ = $2; }
    | LOCATION               { $$ = ast_new_identifier($1, @$); }
    | NUMBER                 { $$ = ast_new_number($1, @$); }
    | TRUE                   { $$ = ast_new_number(1, @$); }
    | FALSE                  { $$ = ast_new_number(0, @$); }
    | METHOD '(' ACTUALS ')' { $$ = ast_new_call($1, ast_list_reverse($3), @$); }
    ;

ACTUALS:
      /* empty */   { $$ = NULL; }
    | ARGS EXPR     { $$ = ast_list_prepend($1, $2); }
    ;

ARGS:
      /* empty */   { $$ = NULL; }
    | ARGS EXPR ',' { $$ = ast_list_prepend($1, $2); }
    ;

%%

void yyerror(ASTNode **ast, const char *s) {
  extern char *yytext;
  extern YYLTYPE yylloc;
  fprintf(stderr, "error: %s near '%s' at line %d, column %d\n",
          s, yytext, yylloc.first_line, yylloc.first_column);
  fprintf(stderr, "\n");
}

