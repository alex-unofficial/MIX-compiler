#include "table.h"
#include "label.h"

#include <stdio.h>

unsigned int ht_from_ast(const ASTNode *n, HashTable *gt, HashTable *lt, const char *scope) {
  if (n == NULL) return 0;

  static unsigned int param_count;
  static unsigned int local_count;

  static enum DataType decl_type;

  TableEntry *e = NULL;
  unsigned int semantic_errors = 0;

  switch (n->kind) {
    case N_PROGRAM:
      return ht_from_ast_list(n->prog.methods, gt, NULL, NULL);

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

        param_count = 0;
        local_count = 0;

        semantic_errors += ht_from_ast_list(n->method.params, gt, st, n->method.name);
        semantic_errors += ht_from_ast(n->method.body, gt, st, n->method.name);

        Payload method_payload = {
          .kind = PAYLOAD_METHOD,
          .loc = n->loc,
          .method = {
            .return_type = n->method.type,
            .param_count = param_count,
            .local_count = local_count,
            .symbols = st,
            .label = gen_method_label()
          }
        };

        ht_add_entry(gt, n->method.name, method_payload);
        
        return semantic_errors;
      }

    case N_PARAM:
      param_count += 1;

      e = ht_find_entry(lt, n->param.name);
      if (e != NULL) {
        fprintf(stderr, "In method '%s':\n", scope);
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
            .offset = param_count + 1
          }
        };

        ht_add_entry(lt, n->param.name, param_payload);
        return 0;
      }

    case N_BODY:
      semantic_errors += ht_from_ast_list(n->body.decls, gt, lt, scope);
      semantic_errors += ht_from_ast_list(n->body.stmts, gt, lt, scope);
      return semantic_errors;

    case N_DECL:
      decl_type = n->decl.type;
      return ht_from_ast_list(n->decl.vars, gt, lt, scope);

    case N_VAR:
      local_count += 1;
      semantic_errors += ht_from_ast(n->var.expr, gt, lt, scope);

      e = ht_find_entry(lt, n->var.name);
      if (e != NULL) {
        fprintf(stderr, "In method '%s':\n", scope);
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
            .symbol_type = decl_type,
            .kind = SYMBOL_LOCAL,
            .offset = -local_count
          }
        };

        ht_add_entry(lt, n->var.name, local_payload);
      }
      return semantic_errors;

    case N_BLOCK:
      return ht_from_ast_list(n->block.stmts, gt, lt, scope);

    case N_ASSIGN:
      e = ht_find_entry(lt, n->assign.location);
      if (e == NULL) {
        semantic_errors += 1;
        fprintf(stderr, "In method '%s':\n", scope);
        fprintf(stderr, "error: variable '%s' at line %d, column %d not declared in scope\n",
            n->assign.location, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
      }
      semantic_errors += ht_from_ast(n->assign.rhs, gt, lt, scope);
      return semantic_errors;

    case N_IF:
      semantic_errors += ht_from_ast(n->branch.cond, gt, lt, scope);
      semantic_errors += ht_from_ast(n->branch.then_branch, gt, lt, scope);
      semantic_errors += ht_from_ast(n->branch.else_branch, gt, lt, scope);
      return semantic_errors;

    case N_WHILE:
      semantic_errors += ht_from_ast(n->branch.cond, gt, lt, scope);
      semantic_errors += ht_from_ast(n->branch.then_branch, gt, lt, scope);
      return semantic_errors;

    case N_RETURN:
      return ht_from_ast(n->ret.expr, gt, lt, scope);

    case N_BREAK:
      return 0;

    case N_EXPR:
      semantic_errors += ht_from_ast(n->binop.lhs, gt, lt, scope);
      semantic_errors += ht_from_ast(n->binop.rhs, gt, lt, scope);
      return semantic_errors;

    case N_CALL:
      e = ht_find_entry(gt, n->call.fname);
      if (e == NULL) {
        semantic_errors += 1;
        fprintf(stderr, "In method '%s':\n", scope);
        fprintf(stderr, "error: method '%s' at line %d, column %d not declared\n",
            n->call.fname, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
      }
      semantic_errors += ht_from_ast_list(n->call.args, gt, lt, scope);
      return semantic_errors;

    case N_IDENTIFIER:
      e = ht_find_entry(lt, n->identifier.name);
      if (e == NULL) {
        fprintf(stderr, "In method '%s':\n", scope);
        fprintf(stderr, "error: variable '%s' at line %d, column %d not declared in scope\n",
            n->identifier.name, n->loc.first_line, n->loc.first_column);
        fprintf(stderr, "\n");
        return 1;
      } else return 0;

    case N_NUMBER:
      return 0;
      if (n == NULL) return 0;

    default:
        fprintf(stderr, "internal error: unknown node kind %d\n", n->kind);
        return 1;
  }
}

unsigned int ht_from_ast_list(const ASTList *l, HashTable *gt, HashTable *lt, const char *scope) {
  const ASTList *i = l;
  unsigned int semantic_errors = 0;
  while (i != NULL) {
    semantic_errors += ht_from_ast(i->node, gt, lt, scope);
    i = i->list;
  }

  return semantic_errors;
}
