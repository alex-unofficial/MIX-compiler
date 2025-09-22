#ifndef GEN_H
#define GEN_H

#include "ast.h"
#include "table.h"

#define REG_SP 6
#define REG_FP 5

#define STR(x) #x
#define CHAR(x) ('0' + (x))
#define INC(reg) "INC" STR(reg)
#define DEC(reg) "DEC" STR(reg)
#define ENT(reg) "ENT" STR(reg)
#define LD(reg)  "LD"  STR(reg)
#define ST(reg)  "ST"  STR(reg)

/* generate assembly from AST */
int gen_mixal_from_ast(const ASTNode *root, const HashTable *gt);

/* program skeleton */
int gen_program_prologue(const char *entry_label, const char *main_label, unsigned int origin);
int gen_program_epilogue(const char *entry_label);

/* subroutine opetations */
int gen_method_entry(const char *method_name, const char *label, unsigned int n_locals);
int gen_method_exit(const char *method_name, unsigned int n_params);
int gen_method_return();
int gen_method_call(const char *method_name, const char *label);

/* branch operations */
int gen_branch_label(const char *l_label);          // initialize branch label
int gen_branch_entry(const char *l_break);          // set jump on condition fail
int gen_branch_jmp(const char *l_branch);           // set jump to branch continue
int gen_branch_break(const char *l_done);           // set jump to loop break

/* stack insertion, deletion opetations */
int gen_push_var(const char *var_name, int offset); // push variable to stack
int gen_pop_var(const char *var_name, int offset);  // pop variable from stack
int gen_push_num(int value);                        // push number to stack

/* numerical and logical operations */
int gen_unary_neg(); // unary (-) operation
int gen_binop_add(); // binary (+) operation
int gen_binop_sub(); // binary (-) operation
int gen_binop_mul(); // binary (*) operation
int gen_binop_div(); // binary (/) operation
int gen_relop_leq(); // relation (<=) operation
int gen_relop_lt();  // relation (<) operation
int gen_relop_gt();  // relation (>) operation
int gen_relop_geq(); // relation (>=) operation
int gen_relop_eq();  // relation (==) operation
int gen_relop_neq(); // relation (!=) operation

#endif
