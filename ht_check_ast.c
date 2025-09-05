#include "table.h"

#include <stdio.h>

unsigned int ht_check_ast(const ASTNode *n, const HashTable *gt, 
													const HashTable *lt, const char *scope) {
	if (n == NULL) return 0;

	TableEntry *e = NULL;
	unsigned int total_errors = 0;

	switch (n->kind) {
		case N_PROGRAM:
			return ht_check_ast_list(n->prog.methods, gt, NULL, NULL);

		case N_METHOD:
			e = ht_find_entry(gt, n->method.name);
			if (e == NULL) {
				fprintf(stderr, "implementation error: method '%s' at line %d not in symbol table\n",
								n->method.name, n->loc.first_line);
				fprintf(stderr, "\n");

				return 1;

			} else {
				total_errors += ht_check_ast_list(n->method.params, gt, 
																					e->payload.method.symbols, n->method.name);
				total_errors += ht_check_ast(n->method.body, gt, 
																		 e->payload.method.symbols, n->method.name);
				
				return total_errors;
			}

		case N_PARAM:
			e = ht_find_entry(lt, n->param.name);
			if (e == NULL) {
				fprintf(stderr, "In method '%s':\n", scope);
				fprintf(stderr, "implementation error: param '%s' at line %d, column %d not in symbol table\n",
								n->param.name, n->loc.first_line, n->loc.first_column);
				fprintf(stderr, "\n");
				return 1;
			} else return 0;

		case N_BODY:
			total_errors += ht_check_ast_list(n->body.decls, gt, lt, scope);
			total_errors += ht_check_ast_list(n->body.stmts, gt, lt, scope);
			return total_errors;

		case N_DECL:
			return ht_check_ast_list(n->decl.vars, gt, lt, scope);

		case N_VAR:
			e = ht_find_entry(lt, n->var.name);
			if (e == NULL) {
				total_errors += 1;
				fprintf(stderr, "In method '%s':\n", scope);
				fprintf(stderr, "implementation error: local '%s' at line %d, column %d not in symbol table\n",
								n->var.name, n->loc.first_line, n->loc.first_column);
				fprintf(stderr, "\n");
			}
			total_errors += ht_check_ast(n->var.expr, gt, lt, scope);
			return total_errors;

		case N_BLOCK:
			return ht_check_ast_list(n->block.stmts, gt, lt, scope);

		case N_ASSIGN:
			e = ht_find_entry(lt, n->assign.location);
			if (e == NULL) {
				total_errors += 1;
				fprintf(stderr, "In method '%s':\n", scope);
				fprintf(stderr, "error: variable '%s' at line %d, column %d not declared in scope\n",
						n->assign.location, n->loc.first_line, n->loc.first_column);
				fprintf(stderr, "\n");
			}
			total_errors += ht_check_ast(n->assign.rhs, gt, lt, scope);
			return total_errors;

		case N_IF:
			total_errors += ht_check_ast(n->branch.cond, gt, lt, scope);
			total_errors += ht_check_ast(n->branch.then_branch, gt, lt, scope);
			total_errors += ht_check_ast(n->branch.else_branch, gt, lt, scope);
			return total_errors;

		case N_WHILE:
			total_errors += ht_check_ast(n->branch.cond, gt, lt, scope);
			total_errors += ht_check_ast(n->branch.then_branch, gt, lt, scope);
			return total_errors;

		case N_RETURN:
			return ht_check_ast(n->ret.expr, gt, lt, scope);

		case N_BREAK:
			return 0;

		case N_EXPR:
			total_errors += ht_check_ast(n->binop.lhs, gt, lt, scope);
			total_errors += ht_check_ast(n->binop.rhs, gt, lt, scope);
			return total_errors;

		case N_CALL:
			e = ht_find_entry(gt, n->call.fname);
			if (e == NULL) {
				total_errors += 1;
				fprintf(stderr, "In method '%s':\n", scope);
				fprintf(stderr, "error: method '%s' at line %d, column %d not declared\n",
						n->call.fname, n->loc.first_line, n->loc.first_column);
				fprintf(stderr, "\n");
			}
			total_errors += ht_check_ast_list(n->call.args, gt, lt, scope);
			return total_errors;

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

unsigned int ht_check_ast_list(const ASTList *l, const HashTable *gt, 
															 const HashTable *lt, const char *scope) {
	const ASTList *i = l;
	unsigned int total_errors = 0;
	while (i != NULL) {
		total_errors += ht_check_ast(i->node, gt, lt, scope);
		i = i->list;
	}

	return total_errors;
}
