#include "parser.tab.h"
#include "ast.h"
#include "table.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef DEBUG
#define DEBUG true
#endif

#define exit_if(cond) {                                       \
  if (cond) {                                                 \
    fprintf(stderr, "Compilation terminated with errors.\n"); \
    exit(1);                                                  \
  }                                                           \
}

int main() {
  ASTNode *ast_root = NULL;
  exit_if (yyparse(&ast_root))

  if (DEBUG) printf("\n");
  if (DEBUG) printf("SYNTAX TREE:\n");
  if (DEBUG) printf("-----------\n");
  if (DEBUG) ast_print(ast_root, 0);
  if (DEBUG) printf("\n");

  HashTable *function_table = ht_new(TABLE_SIZE);
  exit_if (ht_from_ast(ast_root, function_table))

  if (DEBUG) printf("SYMBOL TABLE:\n");
  if (DEBUG) printf("------------\n");
  if (DEBUG) ht_print(function_table);
  if (DEBUG) printf("\n");

  exit_if (ht_check_ast(ast_root, function_table, NULL, NULL))

  ht_free(function_table);
  ast_free(ast_root);

  return 0;
}
