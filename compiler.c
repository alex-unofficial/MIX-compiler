#include "parser.tab.h"
#include "ast.h"
#include "table.h"

#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

extern unsigned int semantic_errors;

int main() {
	ASTNode *ast_root = NULL;
	if (yyparse(&ast_root)) {
		fprintf(stderr, "Parsing exited with errors.\n");
		exit(1);
	}

	HashTable *function_table = symbols_from_ast(ast_root);
	if (semantic_errors > 0) {
		fprintf(stderr, "Semantic analysis exited with errors.\n");
		exit(1);
	}

	if (DEBUG) ast_print(ast_root, 0);

	free_ht(function_table);
	ast_free(ast_root); // clean up
	return 0;
}
