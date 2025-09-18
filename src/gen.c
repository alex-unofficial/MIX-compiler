#include "gen.h"
#include "emit.h"

static int gen_push_reg(char reg) {
  char inst[4];
  char address[32];
  char comment[64];

  const char *reg_str = (reg >= '1' && reg <= '6') ? "rI" : "r";

  // increment SP
  if (emit_inst(NULL, INC(REG_SP), "1", "SP ← SP + 1")) return -1;

  // push register to stack (store at top of stack)
  if (snprintf(inst, sizeof(inst), "ST%c", reg)
      >= (int)sizeof(inst)) return -1;
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP) 
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "STACK[SP] ← %s%c", reg_str, reg) 
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, inst, address, comment)) return -1;

  return 0;
}

static int gen_pop_reg(char reg) {
  char inst[4];
  char address[32];
  char comment[64];

  const char *reg_str = (reg >= '1' && reg <= '6') ? "rI" : "r";

  // pop register from stack (load from top of stack)
  if (snprintf(inst, sizeof(inst), "LD%c", reg)
      >= (int)sizeof(inst)) return -1;
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP) 
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "%s%c ← STACK[SP]", reg_str, reg) 
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, inst, address, comment)) return -1;

  // decrement SP
  if (emit_inst(NULL, DEC(REG_SP), "1", "SP ← SP - 1")) return -1;

  return 0;
}

int gen_push_var(const char *var_name, int offset) {
  char address[32];
  char comment[64];

  emit_comment("Push %s to stack", var_name);
  
  // load STACK[FP + offset] to rA
  if (snprintf(address, sizeof(address), "STACK%+d,%u", offset, REG_FP)
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "rA ← %s ≡ STACK[FP%+d]", var_name, offset)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "LDA", address, comment)) return -1;

  // push rA to stack
  if (gen_push_reg('A')) return -1;
  
  return 0;
}

int gen_push_num(int value) {
  char address[32];
  char comment[64];

  emit_comment("Push %d to stack", value);

  // load literal value to rA
  if (snprintf(address, sizeof(address), "=%d=", value)
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "rA ← %d", value)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "LDA", address, comment)) return -1;

  // push rA to stack
  if (gen_push_reg('A')) return -1;

  return 0;
}

int gen_pop_var(const char *var_name, int offset) {
  char address[32];
  char comment[64];

  emit_comment("Pop %s from stack", var_name);
  
  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // store rA to STACK[FP + offset]
  if (snprintf(address, sizeof(address), "STACK%+d,%u", offset, REG_FP)
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "STACK[FP%+d] ≡ %s ← rA", offset, var_name)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "STA", address, comment)) return -1;
  
  return 0;
}


