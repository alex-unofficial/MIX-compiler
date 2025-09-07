#include "parser.tab.h"
#include "ast.h"
#include "table.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef DEBUG
#define DEBUG true
#endif

// bison and flex input file.
extern FILE *yyin;

int set_input_file(const char *fname, FILE **fp);

#define exit_if(cond) {                                       \
  if (cond) {                                                 \
    fprintf(stderr, "Compilation terminated with errors.\n"); \
    exit(1);                                                  \
  }                                                           \
}

int main(int argc, char ** argv) {
  char *ifname = NULL;
  if (argc > 1) {
    ifname = argv[1];
    exit_if (set_input_file(ifname, &yyin));
  }

  ASTNode *ast_root = NULL;
  exit_if (yyparse(&ast_root));

  if (DEBUG) printf("\n");
  if (DEBUG) printf("SYNTAX TREE:\n");
  if (DEBUG) printf("-----------\n");
  if (DEBUG) ast_print(ast_root, 0);
  if (DEBUG) printf("\n");

  HashTable *function_table = ht_new(TABLE_SIZE);
  exit_if (ht_from_ast(ast_root, function_table));

  if (DEBUG) printf("SYMBOL TABLE:\n");
  if (DEBUG) printf("------------\n");
  if (DEBUG) ht_print(function_table);
  if (DEBUG) printf("\n");

  exit_if (ht_check_ast(ast_root, function_table, NULL, NULL));

  ht_free(function_table);
  ast_free(ast_root);

  return 0;
}

int set_input_file(const char *fname, FILE **fp) {
  FILE *f = fopen(fname, "r");

  if (f == NULL) {
    int errsv = errno;
    fprintf(stderr, "error: cannot open file '%s':\n", fname);
    switch (errsv) {
      case ENOENT:
        fprintf(stderr, "No such file or directory.\n");
        break;

      default:
        fprintf(stderr, "Error code: %d\n", errsv);
        break;
    }
    fprintf(stderr, "\n");
    return -1;
  }

  struct stat sb;
  if (fstat(fileno(f), &sb) == -1) {
    fprintf(stderr, "internal error: cannot create file status buffer.\n");
    fprintf(stderr, "\n");

    fclose(f);
    return -1;
  }

  if (S_ISDIR(sb.st_mode)) {
    fprintf(stderr, "error: '%s' is a directory.\n", fname);
    fprintf(stderr, "\n");

    fclose(f);
    return -1;
  }

  *fp = f;

  return 0;
}
