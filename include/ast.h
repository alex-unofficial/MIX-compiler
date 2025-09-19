#ifndef AST_H
#define AST_H

#include "parser.tab.h"
#include <stdlib.h>

typedef struct ASTNode ASTNode;
typedef struct ASTList ASTList;

enum NodeKind {
  N_PROGRAM,
  N_METHOD,
  N_PARAM,
  N_BODY,
  N_DECL,
  N_VAR,
  N_BLOCK,
  N_ASSIGN,
  N_IF,
  N_WHILE,
  N_RETURN,
  N_BREAK,
  N_BINOP,
  N_UNARY,
  N_CALL,
  N_IDENTIFIER,
  N_NUMBER
};

enum OpKind {
  OP_RELOP_LEQ, OP_RELOP_LT,
  OP_RELOP_GT, OP_RELOP_GEQ,
  OP_RELOP_EQ, OP_RELOP_NEQ,
  OP_ADDOP_ADD, OP_ADDOP_SUB,
  OP_MULOP_MUL, OP_MULOP_DIV
};

static const char *op_kind_str[] = {
  [OP_RELOP_LEQ] = "<=",
  [OP_RELOP_LT]  = "<",
  [OP_RELOP_GT]  = ">",
  [OP_RELOP_GEQ] = ">=",
  [OP_RELOP_EQ]  = "==",
  [OP_RELOP_NEQ] = "!=",
  [OP_ADDOP_ADD] = "+",
  [OP_ADDOP_SUB] = "-",
  [OP_MULOP_MUL] = "*",
  [OP_MULOP_DIV] = "/"
}; 

enum DataType {
  TYPE_INT
};

static const char *data_type_str[] = {
  [TYPE_INT] = "int"
};

struct ASTList {
  ASTNode *node;
  ASTList *list;
};

struct ASTNode {
  enum NodeKind kind;
  YYLTYPE loc;

  union {
    // PROGRAM: A list of METHODs
    struct { ASTList *methods; } prog;

    // METHOD: has a type, a name, a list of PARAMS and a BODY
    struct { enum DataType type; char *name; ASTList *params; ASTNode *body; } method;

    // PARAM: has a type and an identifier
    struct { enum DataType type; char *name; } param;

    // BODY: is a list of DECLs and a list of STMTs (BLOCK/ASSIGN/IF/WHILE/RETURN/BREAK)
    struct { ASTList *decls; ASTList *stmts; } body;

    // DECL: has a type and a list of VARs
    struct { enum DataType type; ASTList *vars; } decl;

    // VAR: has a name and optionally an initializer
    struct { char *name; ASTNode *expr; } var;

    // BLOCK: a list of statements
    struct { ASTList *stmts; } block;

    // ASSIGN: location identifier and expression
    struct { char *location; ASTNode *rhs; } assign;

    // IF/WHILE: conditional expression and then/else blocks or statements
    struct { ASTNode *cond; ASTNode *then_branch; ASTNode *else_branch; } branch;

    // RETURN: optional expression
    struct { ASTNode *expr; } ret;

    // BINOP: binary operation between lhs and rhs
    struct { enum OpKind op; ASTNode *lhs; ASTNode *rhs; } binop;

    // UNARY: unary operation on expr
    struct { enum OpKind op; ASTNode *expr; } unary;

    // CALL: function name and optional parameters
    struct { char *fname; ASTList *args; } call;

    // IDENTIFIER: an identifier name
    struct { char *name; } identifier;

    // NUMBER: a numeric value
    struct { int val; } number;
  };
};

// helper functions
ASTNode *ast_new_node(enum NodeKind kind, YYLTYPE loc);
ASTList *ast_list_prepend(ASTList *list, ASTNode *node);
ASTList *ast_list_reverse(ASTList *head);
unsigned int ast_list_size(ASTList *head);

void ast_print(ASTNode *n, int indent);
void ast_list_print(ASTList *l, int indent);

// constructors for each type of node
ASTNode *ast_new_number(int val, YYLTYPE loc);
ASTNode *ast_new_identifier(char *name, YYLTYPE loc);
ASTNode *ast_new_call(char *fname, ASTList *args, YYLTYPE loc);
ASTNode *ast_new_unary(enum OpKind op, ASTNode *expr, YYLTYPE loc);
ASTNode *ast_new_binop(enum OpKind op, ASTNode *lhs, ASTNode *rhs, YYLTYPE loc);
ASTNode *ast_new_break(YYLTYPE loc);
ASTNode *ast_new_return(ASTNode *expr, YYLTYPE loc);
ASTNode *ast_new_while(ASTNode *cond, ASTNode *then_branch, YYLTYPE loc);
ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, YYLTYPE loc);
ASTNode *ast_new_assign(char *location, ASTNode *rhs, YYLTYPE loc);
ASTNode *ast_new_block(ASTList *stmts, YYLTYPE loc);
ASTNode *ast_new_var(char *name, ASTNode *expr, YYLTYPE loc);
ASTNode *ast_new_decl(enum DataType type, ASTList *vars, YYLTYPE loc);
ASTNode *ast_new_body(ASTList *decls, ASTList *stmts, YYLTYPE loc);
ASTNode *ast_new_param(enum DataType type, char *name, YYLTYPE loc);
ASTNode *ast_new_method(enum DataType type, char *name,
                        ASTList *params, ASTNode *body, YYLTYPE loc);
ASTNode *ast_new_program(ASTList *methods, YYLTYPE loc);

// recursive free functions
void ast_free(ASTNode *n);
void ast_list_free(ASTList *l);

#endif
