#include "table.h"
#include "label.h"

#include <stdio.h>

struct SymbolTableContext {
  HashTable *lt;
  char *scope;
  unsigned int param_count;
  unsigned int local_count;
  enum DataType decl_type;
};

static unsigned int _ht_from_ast(const ASTNode *n, HashTable *gt, struct SymbolTableContext *ctxt);
static unsigned int _ht_from_ast_list(const ASTList *l, HashTable *gt, struct SymbolTableContext *ctxt);

unsigned int ht_from_ast(const ASTNode *n, HashTable **gt) {
  *gt = ht_new(TABLE_SIZE);

  // TODO: check main function exists

  return _ht_from_ast(n, *gt, NULL);
}

static unsigned int _ht_from_ast(const ASTNode *n, HashTable *gt, struct SymbolTableContext *ctxt) {
  if (n == NULL) return 0;

  TableEntry *e = NULL;
  unsigned int semantic_errors = 0;

  switch (n->kind) {
    case N_PROGRAM:
      return _ht_from_ast_list(n->prog.methods, gt, ctxt);

    case N_METHOD:
      e = ht_find_entry(gt, n->method.name);
      if (e != NULL) {
        fprintf(stderr, "error: method definition '%s' at line %d\n", 
                n->method.name, n->loc.first_line);
        fprintf(stderr, "  conflicts with definition at line %d\n", e->payload.loc.first_line);
        fprintf(stderr, "\n");
        return 1;

      } else {
        HashTable *st = ht_new(TABLE_SIZE);

        struct SymbolTableContext mctxt = {
          .lt = st,
          .scope = n->method.name,
          .param_count = 0,
          .local_count = 0
        };

        semantic_errors += _ht_from_ast_list(n->method.params, gt, &mctxt);
        semantic_errors += _ht_from_ast(n->method.body, gt, &mctxt);

        Payload method_payload = {
          .kind = PAYLOAD_METHOD,
          .loc = n->loc,
          .method = {
            .return_type = n->method.type,
            .param_count = mctxt.param_count,
            .local_count = mctxt.local_count,
            .symbols = st,
            .label = gen_method_label()
          }
        };

        ht_add_entry(gt, n->method.name, method_payload);
        
        return semantic_errors;
      }

    case N_PARAM:
      ctxt->param_count += 1;

      e = ht_find_entry(ctxt->lt, n->param.name);
      if (e != NULL) {
        fprintf(stderr, "In method '%s':\n", ctxt->scope);
        fprintf(stderr, "error: parameter '%s' defined multiple times\n", n->param.name);
        fprintf(stderr, "  at line %d, column %d\n", 
                e->payload.loc.first_line, e->payload.loc.first_column);
        fprintf(stderr, "  and line %d, column %d\n", 
                n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
        return 1;
      } else {
        Payload param_payload = {
          .kind = PAYLOAD_SYMBOL,
          .loc = n->loc,
          .symbol = {
            .symbol_type = n->param.type,
            .kind = SYMBOL_PARAM,
            .offset = ctxt->param_count + 1
          }
        };

        ht_add_entry(ctxt->lt, n->param.name, param_payload);
        return 0;
      }

    case N_BODY:
      semantic_errors += _ht_from_ast_list(n->body.decls, gt, ctxt);
      semantic_errors += _ht_from_ast_list(n->body.stmts, gt, ctxt);
      return semantic_errors;

    case N_DECL:
      ctxt->decl_type = n->decl.type;
      return _ht_from_ast_list(n->decl.vars, gt, ctxt);

    case N_VAR:
      ctxt->local_count += 1;
      semantic_errors += _ht_from_ast(n->var.expr, gt, ctxt);

      e = ht_find_entry(ctxt->lt, n->var.name);
      if (e != NULL) {
        fprintf(stderr, "In method '%s':\n", ctxt->scope);
        fprintf(stderr, "error: variable declaration '%s' at line %d, column %d\n", 
                n->var.name, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "  conflicts with %s definition at line %d, column %d\n", 
                sym_kind_str[e->payload.symbol.kind],
                e->payload.loc.first_line, e->payload.loc.first_column);
        fprintf(stderr, "\n");
        semantic_errors += 1;
      } else {
        Payload local_payload = {
          .kind = PAYLOAD_SYMBOL,
          .loc = n->loc,
          .symbol = {
            .symbol_type = ctxt->decl_type,
            .kind = SYMBOL_LOCAL,
            .offset = -ctxt->local_count
          }
        };

        ht_add_entry(ctxt->lt, n->var.name, local_payload);
      }

      return semantic_errors;

    case N_BLOCK:
      return _ht_from_ast_list(n->block.stmts, gt, ctxt);

    case N_ASSIGN:
      e = ht_find_entry(ctxt->lt, n->assign.location);
      if (e == NULL) {
        semantic_errors += 1;
        fprintf(stderr, "In method '%s':\n", ctxt->scope);
        fprintf(stderr, "error: variable '%s' at line %d, column %d not declared in scope\n",
            n->assign.location, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
      }
      semantic_errors += _ht_from_ast(n->assign.rhs, gt, ctxt);

      return semantic_errors;

    case N_IF:
      semantic_errors += _ht_from_ast(n->branch.cond, gt, ctxt);
      semantic_errors += _ht_from_ast(n->branch.then_branch, gt, ctxt);
      semantic_errors += _ht_from_ast(n->branch.else_branch, gt, ctxt);
      return semantic_errors;

    case N_WHILE:
      semantic_errors += _ht_from_ast(n->branch.cond, gt, ctxt);
      semantic_errors += _ht_from_ast(n->branch.then_branch, gt, ctxt);
      return semantic_errors;

    case N_RETURN:
      return _ht_from_ast(n->ret.expr, gt, ctxt);

    case N_BREAK:
      return 0;

    case N_BINOP:
      semantic_errors += _ht_from_ast(n->binop.lhs, gt, ctxt);
      semantic_errors += _ht_from_ast(n->binop.rhs, gt, ctxt);
      return semantic_errors;

    case N_UNARY:
      return _ht_from_ast(n->unary.expr, gt, ctxt);

    case N_CALL:
      e = ht_find_entry(gt, n->call.fname);
      if (e == NULL) {
        semantic_errors += 1;
        fprintf(stderr, "In method '%s':\n", ctxt->scope);
        fprintf(stderr, "error: method '%s' at line %d, column %d not declared\n",
            n->call.fname, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
      }
      semantic_errors += _ht_from_ast_list(n->call.args, gt, ctxt);
      // TODO: check arg count matches with definition
      return semantic_errors;

    case N_IDENTIFIER:
      e = ht_find_entry(ctxt->lt, n->identifier.name);
      if (e == NULL) {
        fprintf(stderr, "In method '%s':\n", ctxt->scope);
        fprintf(stderr, "error: variable '%s' at line %d, column %d not declared in scope\n",
            n->identifier.name, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
        return 1;
      } else return 0;

    case N_NUMBER:
      return 0;

    default:
        fprintf(stderr, "internal error: unknown node kind %d\n", n->kind);
        return 1;
  }
}

static unsigned int _ht_from_ast_list(const ASTList *l, HashTable *gt, struct SymbolTableContext *ctxt) {
  const ASTList *i = l;
  unsigned int semantic_errors = 0;
  while (i != NULL) {
    semantic_errors += _ht_from_ast(i->node, gt, ctxt);
    i = i->list;
  }

  return semantic_errors;
}
