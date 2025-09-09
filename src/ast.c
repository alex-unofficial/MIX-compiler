#include "ast.h"
#include <stdio.h>
#include <string.h>

ASTNode *ast_new_node(enum NodeKind kind, YYLTYPE loc) {
  ASTNode *n = calloc(1, sizeof(ASTNode));

  n->kind = kind;
  n->loc = loc;

  return n;
}

ASTList *ast_list_prepend(ASTList *list, ASTNode *node) {
  ASTList *l = malloc(sizeof(ASTList));

  l->node = node;
  l->list = list;

  return l;
}

ASTList *ast_list_reverse(ASTList *head) {
  ASTList *prev = NULL;
  ASTList *current = head;
  ASTList *next;

  while(current != NULL) {
    next = current->list;

    current->list = prev;

    prev = current;
    current = next;
  }

  return prev;
}

ASTNode *ast_new_number(int val, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_NUMBER, loc);
  n->number.val = val;
  return n;
}

ASTNode *ast_new_identifier(char *name, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_IDENTIFIER, loc);
  n->identifier.name = name;
  return n;
}

ASTNode *ast_new_call(char *fname, ASTList *args, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_CALL, loc);
  n->call.fname = fname;
  n->call.args = args;
  return n;
}

ASTNode *ast_new_unary(enum OpKind op, ASTNode *expr, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_UNARY, loc);
  n->unary.op = op;
  n->unary.expr = expr;
  return n;
}

ASTNode *ast_new_binop(enum OpKind op, ASTNode *lhs, ASTNode *rhs, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_BINOP, loc);
  n->binop.op = op;
  n->binop.lhs = lhs;
  n->binop.rhs = rhs;
  return n;
}

ASTNode *ast_new_break(YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_BREAK, loc);
  return n;
}

ASTNode *ast_new_return(ASTNode *expr, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_RETURN, loc);
  n->ret.expr = expr;
  return n;
}

ASTNode *ast_new_while(ASTNode *cond, ASTNode *then_branch, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_WHILE, loc);
  n->branch.cond = cond;
  n->branch.then_branch = then_branch;
  n->branch.else_branch = NULL;
  return n;
}

ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_IF, loc);
  n->branch.cond = cond;
  n->branch.then_branch = then_branch;
  n->branch.else_branch = else_branch;
  return n;
}

ASTNode *ast_new_assign(char *location, ASTNode *rhs, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_ASSIGN, loc);
  n->assign.location = location;
  n->assign.rhs = rhs;
  return n;
}

ASTNode *ast_new_block(ASTList *stmts, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_BLOCK, loc);
  n->block.stmts = stmts;
  return n;
}

ASTNode *ast_new_var(char *name, ASTNode *expr, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_VAR, loc);
  n->var.name = name;
  n->var.expr = expr;
  return n;
}

ASTNode *ast_new_decl(enum DataType type, ASTList *vars, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_DECL, loc);
  n->decl.type= type;
  n->decl.vars = vars;
  return n;
}

ASTNode *ast_new_body(ASTList *decls, ASTList *stmts, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_BODY, loc);
  n->body.decls = decls;
  n->body.stmts = stmts;
  return n;
}

ASTNode *ast_new_param(enum DataType type, char *name, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_PARAM, loc);
  n->param.type= type;
  n->param.name = name;
  return n;
}

ASTNode *ast_new_method(
    enum DataType type, char *name, ASTList *params, ASTNode *body, YYLTYPE loc
    ) {
  ASTNode *n = ast_new_node(N_METHOD, loc);
  n->method.type= type;
  n->method.name = name;
  n->method.params = params;
  n->method.body = body;
  return n;
}

ASTNode *ast_new_program(ASTList *methods, YYLTYPE loc) {
  ASTNode *n = ast_new_node(N_PROGRAM, loc);
  n->prog.methods = methods;
  return n;
}

void ast_free(ASTNode *n) {
  if (n != NULL) {
    switch(n->kind) {
      case N_PROGRAM:
        ast_list_free(n->prog.methods);
        break;

      case N_METHOD:
        free(n->method.name);
        ast_list_free(n->method.params);
        ast_free(n->method.body);
        break;

      case N_PARAM:
        free(n->param.name);
        break;

      case N_DECL:
        ast_list_free(n->decl.vars);
        break;

      case N_VAR:
        free(n->var.name);
        ast_free(n->var.expr);
        break;

      case N_BLOCK:
        ast_list_free(n->block.stmts);
        break;

      case N_ASSIGN:
        free(n->assign.location);
        ast_free(n->assign.rhs);
        break;

      case N_IF:
        ast_free(n->branch.else_branch);
      case N_WHILE:
        ast_free(n->branch.cond);
        ast_free(n->branch.then_branch);
        break;

      case N_RETURN:
        ast_free(n->ret.expr);
        break;

      case N_BINOP:
        ast_free(n->binop.lhs);
        ast_free(n->binop.rhs);
        break;

      case N_UNARY:
        ast_free(n->unary.expr);
        break;

      case N_CALL:
        free(n->call.fname);
        ast_list_free(n->call.args);
        break;

      case N_IDENTIFIER:
        free(n->identifier.name);
        break;

      case N_BREAK:
      case N_NUMBER:
        break;

      default:
        // TODO: error status
        break;
    }

    free(n);
  }
}

void ast_list_free(ASTList *l) {
  while (l != NULL) {
    ASTList *next = l->list;

    ast_free(l->node);
    free(l);

    l = next;
  }
}

void print_indent(int indent) {
  for (int i = 0; i < indent; i++) printf("  ");
}

void ast_print(ASTNode *n, int indent) {
  if (n != NULL) {
    switch(n->kind) {
      case N_PROGRAM:
        print_indent(indent); printf("PROGRAM:\n");
        ast_list_print(n->prog.methods, indent + 1);
        break;

      case N_METHOD:
        print_indent(indent); printf("METHOD (%s):\n", n->method.name);
        print_indent(indent); printf("→ RETURN TYPE: %s\n", data_type_str[n->method.type]);
        if (n->method.params) {
          print_indent(indent); printf("→ PARAMS:\n");
          ast_list_print(n->method.params, indent + 2);
        }
        print_indent(indent); printf("→ BODY:\n");
        ast_print(n->method.body, indent + 2);
        break;

      case N_PARAM:
        print_indent(indent); printf("%s %s\n", data_type_str[n->param.type], n->param.name);
        break;

      case N_BODY:
        if (n->body.decls) {
          print_indent(indent); printf("→ DECLS:\n");
          ast_list_print(n->body.decls, indent + 2);
        }
        if (n->body.stmts) {
          print_indent(indent); printf("→ STMTS:\n");
          ast_list_print(n->body.stmts, indent + 2);
        }
        break;

      case N_DECL:
        print_indent(indent); printf("TYPE (%s):\n", data_type_str[n->decl.type]);
        ast_list_print(n->decl.vars, indent + 1);
        break;

      case N_VAR:
        print_indent(indent); printf("VAR %s\n", n->var.name);
        if (n->var.expr) {
          print_indent(indent); printf("→ VALUE:\n");
          ast_print(n->var.expr, indent + 2);
        }
        break;

      case N_BLOCK:
        print_indent(indent); printf("BLOCK:\n");
        ast_list_print(n->block.stmts, indent + 1);
        break;

      case N_ASSIGN:
        print_indent(indent); printf("ASSIGN (%s):\n", n->assign.location);
        ast_print(n->assign.rhs, indent + 1);
        break;

      case N_IF:
        print_indent(indent); printf("IF:\n");
        print_indent(indent); printf("→ CONDITION:\n");
        ast_print(n->branch.cond, indent + 2);
        print_indent(indent); printf("→ THEN:\n");
        ast_print(n->branch.then_branch, indent + 2);
        print_indent(indent); printf("→ ELSE:\n");
        ast_print(n->branch.else_branch, indent + 2);
        break;

      case N_WHILE:
        print_indent(indent); printf("WHILE:\n");
        print_indent(indent); printf("→ CONDITION:\n");
        ast_print(n->branch.cond, indent + 2);
        print_indent(indent); printf("→ DO:\n");
        ast_print(n->branch.then_branch, indent + 2);
        break;

      case N_RETURN:
        print_indent(indent); printf("RETURN:\n");
        ast_print(n->ret.expr, indent + 1);
        break;

      case N_BREAK:
        print_indent(indent); printf("BREAK\n");
        break;

      case N_BINOP:
        print_indent(indent); printf("BINOP (%s):\n", op_kind_str[n->binop.op]);
        print_indent(indent); printf("→ LHS:\n");
        ast_print(n->binop.lhs, indent + 1);
        print_indent(indent); printf("→ RHS:\n");
        ast_print(n->binop.rhs, indent + 2);
        break;

      case N_UNARY:
        print_indent(indent); printf("UNARY (%s):\n", op_kind_str[n->unary.op]);
        print_indent(indent); printf("→ EXPR:\n");
        ast_print(n->unary.expr, indent + 1);
        break;

      case N_CALL:
        print_indent(indent); printf("CALL METHOD (%s):\n", n->call.fname);
        print_indent(indent); printf("→ ARGS:\n");
        ast_list_print(n->call.args, indent + 2);
        break;

      case N_IDENTIFIER:
        print_indent(indent); printf("LOCATION (%s)\n", n->identifier.name);
        break;

      case N_NUMBER:
        print_indent(indent); printf("NUMBER (%d)\n", n->number.val);
        break;
    }
  }
}

void ast_list_print(ASTList *l, int indent) {
  ASTList *i = l;
  while (i != NULL) {
    ast_print(i->node, indent);
    i = i->list;
  }
}

