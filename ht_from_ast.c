#include "table.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ast.h"

#define TABLE_SIZE 32
#define LABEL_LEN 5

extern unsigned int semantic_errors;

char *gen_method_label(unsigned int n) {
	const size_t buf_len = LABEL_LEN + 1;
	const unsigned int hex_digits = LABEL_LEN - 1;

	char *l = malloc(buf_len * sizeof(char));
	if (!l) return NULL;

	uint64_t mask;
	if (hex_digits >= sizeof(mask) * 8 / 4) {
		mask = UINT64_MAX;
	} else {
		mask = ((uint64_t) 1 << (4 * hex_digits)) - 1;
	}

	snprintf(l, buf_len, "F%0*X", (int)hex_digits, (unsigned int)(n & mask));

	return l;
}

HashTable *ht_from_ast(const ASTNode *root) {
	if (root->kind != N_PROGRAM) return NULL;

	// Global symbol table of methods
	HashTable *ft = ht_new(TABLE_SIZE);
	
	ASTList *methods_list = root->prog.methods;
	unsigned int method_count = 0;

	while (methods_list != NULL) {
		method_count += 1;

		ASTNode *method_node = methods_list->node;

		// Symbol table for this method
		HashTable *st = ht_new(TABLE_SIZE);

		// Method parameters
		ASTList *params_list = method_node->method.params;
		unsigned int param_count = 0;

		while (params_list != NULL) {
			param_count += 1;

			ASTNode *param_node = params_list->node;

			char *key = param_node->param.name;
			Payload p = {
				.kind = PAYLOAD_SYMBOL,
				.loc = param_node->loc,
				.symbol = {
					.symbol_type = param_node->param.type,
					.kind = SYMBOL_PARAM,
					.offset = param_count + 1
				}
			};

			TableEntry *s = ht_find_entry(st, key);
			if (s == NULL) {
				ht_add_entry(st, key, p);
			} else {
				semantic_errors += 1;
				fprintf(stderr, "In method '%s':\n", method_node->method.name);
				fprintf(stderr, "error: parameter '%s' defined multiple times\n", 
								param_node->param.name);
				fprintf(stderr, "  at line %d, column %d\n", 
								s->payload.loc.first_line, s->payload.loc.first_column);
				fprintf(stderr, "  and line %d, column %d\n", 
								param_node->loc.first_line, param_node->loc.first_column);
				fprintf(stderr, "\n");
			}

			params_list = params_list->list;
		}

		// Method locals
		ASTNode *method_body = method_node->method.body;
		ASTList *decls_list = method_body->body.decls;
		
		unsigned int local_count = 0;

		while (decls_list != NULL) {

			ASTNode *decl_node = decls_list->node;
			enum DataType decl_type = decl_node->decl.type;
			
			ASTList *vars_list = decl_node->decl.vars;

			while (vars_list != NULL) {
				local_count += 1;

				ASTNode *var_node = vars_list->node;

				char *key = var_node->var.name;
				Payload p = {
					.kind = PAYLOAD_SYMBOL,
					.loc = var_node->loc,
					.symbol = {
						.symbol_type = decl_type,
						.kind = SYMBOL_LOCAL,
						.offset = -local_count
					}
				};

				TableEntry *s = ht_find_entry(st, key);
				if (s == NULL) {
					ht_add_entry(st, key, p);
				} else {
					semantic_errors += 1;
					fprintf(stderr, "In method '%s':\n", method_node->method.name);
					fprintf(stderr, "error: variable declaration '%s' at line %d, column %d\n", 
									var_node->var.name, var_node->loc.first_line, var_node->loc.first_column);
					fprintf(stderr, "  conflicts with %s definition at line %d, column %d\n", 
									sym_kind_str[s->payload.symbol.kind],
									s->payload.loc.first_line, s->payload.loc.first_column);
					fprintf(stderr, "\n");
				}

				vars_list = vars_list->list;
			}
			
			decls_list = decls_list->list;
		}

		char *key = method_node->method.name;
		Payload p = {
			.kind = PAYLOAD_METHOD,
			.loc = method_node->loc,
			.method = {
				.return_type = method_node->method.type,
				.param_count = param_count,
				.local_count = local_count,
				.symbols = st,
				.label = gen_method_label(method_count)
			}
		};

		TableEntry *f = ht_find_entry(ft, key);
		if (f == NULL) {
			ht_add_entry(ft, key, p);
		} else {
			semantic_errors += 1;
			fprintf(stderr, "error: method definition '%s' at line %d\n", 
							method_node->method.name, method_node->loc.first_line);
			fprintf(stderr, "  conflicts with definition at line %d\n", f->payload.loc.first_line);
			fprintf(stderr, "\n");
		}

		methods_list = methods_list->list;
	}

	return ft;
}
