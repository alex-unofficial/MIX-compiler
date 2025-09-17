#ifndef GEN_H
#define GEN_H

#define REG_SP 6
#define REG_FP 5

#define STR(x) #x
#define INC(reg) "INC" STR(reg)
#define DEC(reg) "DEC" STR(reg)
#define ENT(reg) "ENT" STR(reg)


int gen_push_var(const char *var_name, int offset);
int gen_push_num(int value);

int gen_pop_var(const char *var_name, int offset);

#endif
