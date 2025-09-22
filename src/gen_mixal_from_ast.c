#include "gen.h"

#include "ast.h"
#include "table.h"
#include "label.h"

#define ORIGIN_ADDR 3000

unsigned int branch_index = 1;

static int _gen_mixal_from_ast_node(const ASTNode *n, const HashTable *gt, 
                                    const HashTable *lt, const char *break_label);

static int _gen_mixal_from_ast_list(const ASTList *l, const HashTable *gt, 
                                    const HashTable *lt, const char *break_label);

static int _gen_mixal_from_ast_list_reverse(const ASTList *l, const HashTable *gt,
                                            const HashTable *lt, const char *break_label);

int gen_mixal_from_ast(const ASTNode *root, const HashTable *gt) {
  return _gen_mixal_from_ast_node(root, gt, NULL, NULL);
}

static int _gen_mixal_from_ast_node(const ASTNode *n, const HashTable *gt, 
                                   const HashTable *lt, const char *break_label) {
  if (n != NULL) {
    const TableEntry *e = NULL;

    switch(n->kind) {
      case N_PROGRAM:
        e = ht_find_entry(gt, "main");

        if (gen_program_prologue("START", e->payload.method.label, ORIGIN_ADDR)) return -1;
        if (_gen_mixal_from_ast_list(n->prog.methods, gt, NULL, NULL)) return -1;
        if (gen_program_epilogue("START")) return -1;
        break;

      case N_METHOD:
        e = ht_find_entry(gt, n->method.name);

        if (gen_method_entry(n->method.name, e->payload.method.label, 
                             e->payload.method.local_count)) return -1;
        if (_gen_mixal_from_ast_list(n->method.params, gt, 
                                    e->payload.method.symbols, break_label)) return -1;
        if (_gen_mixal_from_ast_node(n->method.body, gt, 
                                    e->payload.method.symbols, break_label)) return -1;
        if (gen_method_exit(n->method.name, e->payload.method.param_count)) return -1;

        break;

      case N_PARAM:
        break;

      case N_BODY:
        if (_gen_mixal_from_ast_list(n->body.decls, gt, lt, break_label)) return -1;
        if (_gen_mixal_from_ast_list(n->body.stmts, gt, lt, break_label)) return -1;
        break;

      case N_DECL:
        if (_gen_mixal_from_ast_list(n->decl.vars, gt, lt, break_label)) return -1;
        break;

      case N_VAR:
        if (n->var.expr != NULL) {
          e = ht_find_entry(lt, n->var.name);

          if (_gen_mixal_from_ast_node(n->var.expr, gt, lt, break_label)) return -1;
          if (gen_pop_var(n->var.name, e->payload.symbol.offset)) return -1;
        }

        break;

      case N_BLOCK:
        if (_gen_mixal_from_ast_list(n->block.stmts, gt, lt, break_label)) return -1;
        break;

      case N_ASSIGN:
        e = ht_find_entry(lt, n->assign.location);

        if (_gen_mixal_from_ast_node(n->assign.rhs, gt, lt, break_label)) return -1;
        if (gen_pop_var(n->assign.location, e->payload.symbol.offset)) return -1;

        break;

      case N_IF:
        char *else_label = label_else(branch_index);
        char *cont_label = label_done(branch_index);

        if (_gen_mixal_from_ast_node(n->branch.cond, gt, lt, break_label)) return -1;

        if (gen_branch_entry(else_label)) return -1;

        if (_gen_mixal_from_ast_node(n->branch.then_branch, gt, lt, break_label)) return -1;

        if (gen_branch_jmp(cont_label)) return -1;
        if (gen_branch_label(else_label)) return -1;

        if (_gen_mixal_from_ast_node(n->branch.else_branch, gt, lt, break_label)) return -1;

        if (gen_branch_label(cont_label)) return -1;

        free(else_label);
        free(cont_label);

        branch_index += 1;

        break;

      case N_WHILE:
        char *loop_label = label_loop(branch_index);
        char *done_label = label_done(branch_index);

        if (gen_branch_label(loop_label)) return -1;
        
        if (_gen_mixal_from_ast_node(n->branch.cond, gt, lt, break_label)) return -1;

        if (gen_branch_entry(done_label)) return -1;

        if (_gen_mixal_from_ast_node(n->branch.then_branch, gt, lt, done_label)) return -1;

        if (gen_branch_jmp(loop_label)) return -1;
        if (gen_branch_label(done_label)) return -1;

        free(loop_label);
        free(done_label);

        branch_index += 1;

        break;

      case N_RETURN:
        if (_gen_mixal_from_ast_node(n->ret.expr, gt, lt, break_label)) return -1;
        if (gen_method_return()) return -1;

        break;

      case N_BREAK:
        if (gen_branch_break(break_label)) return -1;
        break;

      case N_BINOP:
        if (_gen_mixal_from_ast_node(n->binop.rhs, gt, lt, break_label)) return -1;
        if (_gen_mixal_from_ast_node(n->binop.lhs, gt, lt, break_label)) return -1;

        switch (n->binop.op) {
          case OP_RELOP_LEQ:
            if (gen_relop_leq()) return -1;
            break;
          case OP_RELOP_LT:
            if (gen_relop_lt()) return -1;
            break;
          case OP_RELOP_GT:
            if (gen_relop_gt()) return -1;
            break;
          case OP_RELOP_GEQ:
            if (gen_relop_geq()) return -1;
            break;
          case OP_RELOP_EQ:
            if (gen_relop_eq()) return -1;
            break;
          case OP_RELOP_NEQ:
            if (gen_relop_neq()) return -1;
            break;
          case OP_ADDOP_ADD:
            if (gen_binop_add()) return -1;
            break;
          case OP_ADDOP_SUB:
            if (gen_binop_sub()) return -1;
            break;
          case OP_MULOP_MUL:
            if (gen_binop_mul()) return -1;
            break;
          case OP_MULOP_DIV:
            if (gen_binop_div()) return -1;
            break;
          default:
            return -1;
        }

        break;

      case N_UNARY:
        if (_gen_mixal_from_ast_node(n->unary.expr, gt, lt, break_label)) return -1;
        switch (n->unary.op) {
          case OP_ADDOP_SUB:
            if (gen_unary_neg()) return -1;
            break;
          default:
            return -1;
        }

        break;

      case N_CALL:
        e = ht_find_entry(gt, n->call.fname);

        if (_gen_mixal_from_ast_list_reverse(n->call.args, gt, lt, break_label)) return -1;
        if (gen_method_call(n->call.fname, e->payload.method.label)) return -1;

        break;

      case N_IDENTIFIER:
        e = ht_find_entry(lt, n->identifier.name);
        
        if (gen_push_var(n->identifier.name, e->payload.symbol.offset)) return -1;

        break;

      case N_NUMBER:
        if (gen_push_num(n->number.val)) return -1;

        break;

      default:
        return -1;
    }
  }

  return 0;
}

static int _gen_mixal_from_ast_list(const ASTList *l, const HashTable *gt, 
                                    const HashTable *lt, const char *break_label) {
  while (l != NULL) {
    if (_gen_mixal_from_ast_node(l->node, gt, lt, break_label)) return -1;
    l = l->list;
  }

  return 0;
}

static int _gen_mixal_from_ast_list_reverse(const ASTList *l, const HashTable *gt,
                                            const HashTable *lt, const char *break_label) {
  if (l != NULL) {
    if (_gen_mixal_from_ast_list_reverse(l->list, gt, lt, break_label)) return -1;
    return _gen_mixal_from_ast_node(l->node, gt, lt, break_label);
  }

  return 0;
}
