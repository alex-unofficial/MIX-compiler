#ifndef GEN_H
#define GEN_H

#define REG_SP 6
#define REG_FP 5

#define STR(x) #x
#define INC(reg) "INC" STR(reg)
#define DEC(reg) "DEC" STR(reg)
#define ENT(reg) "ENT" STR(reg)


/* stack insertion, deletion opetations */
int gen_push_var(const char *var_name, int offset); // push variable to stack
int gen_pop_var(const char *var_name, int offset);  // pop variable from stack
int gen_push_num(int value);                        // push number to stack

/* subroutine opetations */
int gen_call_meth(const char *method_name, const char *label); // call subroutine

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

// binary operations

#endif
