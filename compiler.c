#include "parser.tab.h"
#include "ast.h"
#include "table.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef DEBUG
#define DEBUG 0
#endif

unsigned int semantic_errors = 0;

int main() {
	ASTNode *ast_root = NULL;
	if (yyparse(&ast_root)) {
		fprintf(stderr, "Compilation exited with errors.\n");
		exit(1);
	}

	if (DEBUG) printf("\n");
	if (DEBUG) printf("SYNTAX TREE:\n");
	if (DEBUG) printf("-----------\n");
	if (DEBUG) ast_print(ast_root, 0);

	HashTable *function_table = ht_from_ast(ast_root);
	if (semantic_errors > 0) {
		fprintf(stderr, "Compilation exited with errors.\n");
		exit(1);
	}

	if (DEBUG) printf("\n");
	if (DEBUG) printf("SYMBOL TABLE:\n");
	if (DEBUG) printf("------------\n");
	if (DEBUG) ht_print(function_table);

	if (ht_check_ast(ast_root, function_table, NULL, NULL) > 0) {
		fprintf(stderr, "Compilation exited with errors.\n");
		exit(1);
	}

	ht_free(function_table);
	ast_free(ast_root);
	return 0;
}
