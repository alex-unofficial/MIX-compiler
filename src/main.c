#include "parser.tab.h"
#include "ast.h"
#include "table.h"
#include "emit.h"
#include "gen.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef DEBUG
#define DEBUG 0
#endif

// bison and flex input file.
extern FILE *yyin;

// global output file
extern FILE *mixout;

int set_input_file(const char *fname, FILE **fp);
int set_output_file(const char *fname, FILE **fp);

#define exit_if(cond) {                                       \
  if (cond) {                                                 \
    fprintf(stderr, "Compilation terminated with errors.\n"); \
    exit(1);                                                  \
  }                                                           \
}

int main(int argc, char ** argv) {
  char *ifname = NULL;
  char *ofname = NULL;
  if (argc > 1) {
    ifname = argv[1];
    if (argc > 2) {
      ofname = argv[2];
    }
  }

  exit_if (set_input_file(ifname, &yyin));
  exit_if (set_output_file(ofname, &mixout));

  ASTNode *ast_root = NULL;
  exit_if (yyparse(&ast_root));

#if DEBUG
  printf("\n");
  printf("SYNTAX TREE:\n");
  printf("-----------\n");
  ast_print(ast_root, 0);
  printf("\n");
#endif

  HashTable *function_table = NULL;
  exit_if (ht_from_ast(ast_root, &function_table));

#if DEBUG
  printf("SYMBOL TABLE:\n");
  printf("------------\n");
  ht_print(function_table);
  printf("\n");
#endif

  exit_if (gen_mixal_from_ast(ast_root, function_table));

  ht_free(function_table);
  ast_free(ast_root);

  return 0;
}

int set_input_file(const char *fname, FILE **fp) {
  if (!fname) return 0;

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

int set_output_file(const char *fname, FILE **fp) {
  if (!fname) {
    *fp = stdout;
    return 0;
  }

  FILE *f = fopen(fname, "w");

  if (f == NULL) {
    int errsv = errno;
    fprintf(stderr, "error: cannot open or create file '%s':\n", fname);
    switch (errsv) {
      case ENOENT:
        fprintf(stderr, "File path invalid.\n");
        break;

      case EACCES:
        fprintf(stderr, "Not permitted.\n");
        break;

      default:
        fprintf(stderr, "Error code: %d\n", errsv);
        break;
    }
    fprintf(stderr, "\n");
    return -1;
  }

  *fp = f;

  return 0;
}
