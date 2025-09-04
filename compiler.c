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

	if (DEBUG) printf("\n");
	if (DEBUG) printf("SYNTAX TREE:\n");
	if (DEBUG) printf("-----------\n");
	if (DEBUG) ast_print(ast_root, 0);

	HashTable *function_table = ht_from_ast(ast_root);

	if (DEBUG) printf("\n");
	if (DEBUG) printf("SYMBOL TABLE:\n");
	if (DEBUG) printf("------------\n");
	if (DEBUG) ht_print(function_table);

	if (semantic_errors > 0) {
		fprintf(stderr, "\n");
		fprintf(stderr, "Semantic analysis exited with errors.\n");
		exit(1);
	}


	ht_free(function_table);
	ast_free(ast_root);
	return 0;
}
