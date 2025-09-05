#include "table.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ast.h"
#include "label.h"

unsigned int ht_from_ast(const ASTNode *root, HashTable *ft) {
  if (ft == NULL || root == NULL || root->kind != N_PROGRAM) return 1;

  unsigned int semantic_errors = 0;

  const ASTList *methods_list = root->prog.methods;

  while (methods_list != NULL) {
    const ASTNode *method_node = methods_list->node;

    char *key = method_node->method.name;

    TableEntry *f = ht_find_entry(ft, key);
    if (f == NULL) {
      // Symbol table for this method
      HashTable *st = ht_new(TABLE_SIZE);

      // Method parameters
      ASTList *params_list = method_node->method.params;
      unsigned int param_count = 0;

      while (params_list != NULL) {
        param_count += 1;

        ASTNode *param_node = params_list->node;

        char *key = param_node->param.name;

        TableEntry *s = ht_find_entry(st, key);
        if (s == NULL) {
          Payload p = {
            .kind = PAYLOAD_SYMBOL,
            .loc = param_node->loc,
            .symbol = {
              .symbol_type = param_node->param.type,
              .kind = SYMBOL_PARAM,
              .offset = param_count + 1
            }
          };

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

          TableEntry *s = ht_find_entry(st, key);
          if (s == NULL) {
            Payload p = {
              .kind = PAYLOAD_SYMBOL,
              .loc = var_node->loc,
              .symbol = {
                .symbol_type = decl_type,
                .kind = SYMBOL_LOCAL,
                .offset = -local_count
              }
            };

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

      Payload p = {
        .kind = PAYLOAD_METHOD,
        .loc = method_node->loc,
        .method = {
          .return_type = method_node->method.type,
          .param_count = param_count,
          .local_count = local_count,
          .symbols = st,
          .label = gen_method_label()
        }
      };

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

  return semantic_errors;
}
