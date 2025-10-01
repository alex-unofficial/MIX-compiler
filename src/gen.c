#include "gen.h"
#include "emit.h"

#ifndef ASCII
#define ASCII 0
#endif

#if ASCII
#define SYMB_ASSIGN "<-"
#define SYMB_EQUIV ":="
#else
#define SYMB_ASSIGN "←"
#define SYMB_EQUIV "≡"
#endif

static int gen_push_reg(char reg) {
  char inst[4];
  char address[32];
  char comment[64];

  const char *reg_str = (reg >= '1' && reg <= '6') ? "rI" : "r";

  // increment SP
  if (emit_inst(NULL, INC(REG_SP), "1", "SP " SYMB_ASSIGN " SP + 1")) return -1;

  // TODO: handle stack overflow

  // push register to stack (store at top of stack)
  if (snprintf(inst, sizeof(inst), "ST%c", reg)
      >= (int)sizeof(inst)) return -1;
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP) 
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "STACK[SP] " SYMB_ASSIGN " %s%c", reg_str, reg) 
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, inst, address, comment)) return -1;

  return 0;
}

static int gen_pop_reg(char reg) {
  char inst[4];
  char address[32];
  char comment[64];

  const char *reg_str = (reg >= '1' && reg <= '6') ? "rI" : "r";

  // TODO: handle stack underflow

  // pop register from stack (load from top of stack)
  if (snprintf(inst, sizeof(inst), "LD%c", reg)
      >= (int)sizeof(inst)) return -1;
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP) 
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "%s%c " SYMB_ASSIGN " STACK[SP]", reg_str, reg) 
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, inst, address, comment)) return -1;

  // decrement SP
  if (emit_inst(NULL, DEC(REG_SP), "1", "SP " SYMB_ASSIGN " SP - 1")) return -1;

  return 0;
}

int gen_push_var(const char *var_name, int offset) {
  char address[32];
  char comment[64];

  emit_comment("Push %s to stack", var_name);
  
  // load STACK[FP + offset] to rA
  if (snprintf(address, sizeof(address), "STACK%+d,%u", offset, REG_FP)
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "rA " SYMB_ASSIGN " %s " SYMB_EQUIV " STACK[FP%+d]", var_name, offset)
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
  if (snprintf(comment, sizeof(comment), "rA " SYMB_ASSIGN " %d", value)
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
  if (snprintf(comment, sizeof(comment), "STACK[FP%+d] " SYMB_EQUIV " %s " SYMB_ASSIGN " rA", offset, var_name)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "STA", address, comment)) return -1;
  
  return 0;
}

int gen_unary_neg() {
  char address[32];

  emit_comment("Negation operation on stack (pop A, push -A)");

  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP) 
      >= (int)sizeof(address)) return -1;
  // pop rA from stack negated
  if (emit_inst(NULL, "LDAN", address, "rA " SYMB_ASSIGN " -STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement then increment SP by 1 here. 
     But that would be pointless to emulate and waste instruction cycles. */

  // store rA to STACK[SP]
  if (emit_inst(NULL, "STA", address, "STACK[SP] " SYMB_ASSIGN " rA")) return -1;

  return 0;
}

int gen_binop_add() {
  char address[32];

  emit_comment("Addition operation on stack (pop A, pop B, push A + B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and add to rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "ADD", address, "rA " SYMB_ASSIGN " rA + STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement then increment SP by 1 here. 
     But that would be pointless to emulate and waste instruction cycles. */
  
  // write rA back to the stack
  if (emit_inst(NULL, "STA", address, "STACK[SP] " SYMB_ASSIGN " rA")) return -1;

  return 0;
}

int gen_binop_sub() {
  char address[32];

  emit_comment("Subtraction operation on stack (pop A, pop B, push A - B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and subtract from rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "SUB", address, "rA " SYMB_ASSIGN " rA - STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement then increment SP by 1 here. 
     But that would be pointless to emulate and waste instruction cycles. */
  
  // write rA back to the stack
  if (emit_inst(NULL, "STA", address, "STACK[SP] " SYMB_ASSIGN " rA")) return -1;

  return 0;
}

int gen_binop_mul() {
  char address[32];

  emit_comment("Multiplication operation on stack (pop A, pop B, push A * B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and multiply with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "MUL", address, "rAX " SYMB_ASSIGN " rA * STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement then increment SP by 1 here. 
     But that would be pointless to emulate and waste instruction cycles. */

  // write rX (low word of rAX) back to the stack
  if (emit_inst(NULL, "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_binop_div() {
  char address[32];

  emit_comment("Division operation on stack (pop A, pop B, push A / B)");

  // pop rAX from stack
  if (emit_inst(NULL, "ENTA", "0", "rA <- 0")) return -1;
  if (gen_pop_reg('X')) return -1;

  // pop from stack and divide rAX
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "DIV", address, "rA " SYMB_ASSIGN " rAX / STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement then increment SP by 1 here. 
     But that would be pointless to emulate and waste instruction cycles. */

  // write rA (division result) back to the stack
  if (emit_inst(NULL, "STA", address, "STACK[SP] " SYMB_ASSIGN " rA")) return -1;

  return 0;
}

int gen_relop_leq() {
  char address[32];

  emit_comment("Comparison operation (<=) on stack (pop A, pop B, push A <= B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and compare with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "CMPA", address, "CI " SYMB_ASSIGN " rA ? STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement SP by 1 here (...) */   

  if (emit_inst(NULL, "ENTX", "1", "rX " SYMB_ASSIGN " 1")) return -1;
  if (emit_inst(NULL, "JLE", "1F", "lhs <= rhs? continue")) return -1;
  if (emit_inst(NULL, "ENTX", "0", "else, overwrite rX " SYMB_ASSIGN " 0")) return -1;

  /* (...) then increment SP by 1 here.
     But that would be pointless to emulate and waste instruction cycles. */

  if (emit_inst("1H", "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_relop_lt() {
  char address[32];

  emit_comment("Comparison operation (<) on stack (pop A, pop B, push A < B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and compare with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "CMPA", address, "CI " SYMB_ASSIGN " rA ? STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement SP by 1 here (...) */   

  if (emit_inst(NULL, "ENTX", "1", "rX " SYMB_ASSIGN " 1")) return -1;
  if (emit_inst(NULL, "JL",  "1F", "lhs < rhs? continue")) return -1;
  if (emit_inst(NULL, "ENTX", "0", "else, overwrite rX " SYMB_ASSIGN " 0")) return -1;

  /* (...) then increment SP by 1 here.
     But that would be pointless to emulate and waste instruction cycles. */

  if (emit_inst("1H", "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_relop_gt() {
  char address[32];

  emit_comment("Comparison operation (>) on stack (pop A, pop B, push A > B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and compare with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "CMPA", address, "CI " SYMB_ASSIGN " rA ? STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement SP by 1 here (...) */   

  if (emit_inst(NULL, "ENTX", "1", "rX " SYMB_ASSIGN " 1")) return -1;
  if (emit_inst(NULL, "JG",  "1F", "lhs > rhs? continue")) return -1;
  if (emit_inst(NULL, "ENTX", "0", "else, overwrite rX " SYMB_ASSIGN " 0")) return -1;

  /* (...) then increment SP by 1 here.
     But that would be pointless to emulate and waste instruction cycles. */

  if (emit_inst("1H", "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_relop_geq() {
  char address[32];

  emit_comment("Comparison operation (>=) on stack (pop A, pop B, push A >= B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and compare with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "CMPA", address, "CI " SYMB_ASSIGN " rA ? STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement SP by 1 here (...) */   

  if (emit_inst(NULL, "ENTX", "1", "rX " SYMB_ASSIGN " 1")) return -1;
  if (emit_inst(NULL, "JGE", "1F", "lhs >= rhs? continue")) return -1;
  if (emit_inst(NULL, "ENTX", "0", "else, overwrite rX " SYMB_ASSIGN " 0")) return -1;

  /* (...) then increment SP by 1 here.
     But that would be pointless to emulate and waste instruction cycles. */

  if (emit_inst("1H", "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_relop_eq() {
  char address[32];

  emit_comment("Comparison operation (==) on stack (pop A, pop B, push A == B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and compare with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "CMPA", address, "CI " SYMB_ASSIGN " rA ? STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement SP by 1 here (...) */   

  if (emit_inst(NULL, "ENTX", "1", "rX " SYMB_ASSIGN " 1")) return -1;
  if (emit_inst(NULL, "JE",  "1F", "lhs == rhs? continue")) return -1;
  if (emit_inst(NULL, "ENTX", "0", "else, overwrite rX " SYMB_ASSIGN " 0")) return -1;

  /* (...) then increment SP by 1 here.
     But that would be pointless to emulate and waste instruction cycles. */

  if (emit_inst("1H", "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_relop_neq() {
  char address[32];

  emit_comment("Comparison operation (!=) on stack (pop A, pop B, push A != B)");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // pop from stack and compare with rA
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "CMPA", address, "CI " SYMB_ASSIGN " rA ? STACK[SP]")) return -1;

  /* if this were a real stack machine, 
     it would decrement SP by 1 here (...) */   

  if (emit_inst(NULL, "ENTX", "1", "rX " SYMB_ASSIGN " 1")) return -1;
  if (emit_inst(NULL, "JNE", "1F", "lhs != rhs? continue")) return -1;
  if (emit_inst(NULL, "ENTX", "0", "else, overwrite rX " SYMB_ASSIGN " 0")) return -1;

  /* (...) then increment SP by 1 here.
     But that would be pointless to emulate and waste instruction cycles. */

  if (emit_inst("1H", "STX", address, "STACK[SP] " SYMB_ASSIGN " rX")) return -1;

  return 0;
}

int gen_branch_label(const char *label) {
  // create label at location
  return emit_label(label);
}

int gen_branch_entry(const char *l_break) {
  char comment[64];

  emit_comment("evaluate branch condition");

  // pop rA from stack
  if (gen_pop_reg('A')) return -1;

  // on condition fail, jump to l_break
  if (snprintf(comment, sizeof(comment), "cond = false? jump to %s", l_break)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "JAZ", l_break, comment)) return -1;

  return 0;
}

int gen_branch_jmp(const char *l_continue) {
  char comment[64];

  // jump to l_continue
  if (snprintf(comment, sizeof(comment), "jump to %s", l_continue)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "JMP", l_continue, comment)) return -1;

  return 0;
}

int gen_branch_break(const char *l_done) {
  emit_comment("break from loop (jump to done label)");
  return gen_branch_jmp(l_done);
}

int gen_method_entry(const char *method_name, const char *label, unsigned int n_locals) {
  char address[32];
  char comment[64];

  emit_comment("Subroutine %s entry (store RA & FP, FP " SYMB_ASSIGN " SP, alloc n_locals)",
               method_name);
  
  // push RA to stack
  if (snprintf(address, sizeof(address), "STACK+1,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(label, "STJ", address, "STACK[SP+1] " SYMB_ASSIGN " RA " SYMB_EQUIV " rJ")) return -1;

  // push FP to stack
  if (snprintf(address, sizeof(address), "STACK+2,%u(0:2)", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, ST(REG_FP), address, "STACK[SP+2] " SYMB_ASSIGN " FP")) return -1;

  // set FP " SYMB_ASSIGN " SP
  if (snprintf(address, sizeof(address), "2,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, ENT(REG_FP), address, "FP " SYMB_ASSIGN " SP + 2")) return -1;

  // allocate stack space for n_locals
  if (snprintf(address, sizeof(address), "%u", n_locals + 2)
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "SP " SYMB_ASSIGN " SP + %u", n_locals + 2)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, INC(REG_SP), address, comment)) return -1;

  return 0;
}

int gen_method_exit(const char *method_name, unsigned int n_params) {
  char address[32];
  char comment[64];

  emit_comment("Subroutine %s exit (restore FP & SP, dealloc params, push result, jump to RA)",
               method_name);

  // pop result from stack
  if (snprintf(address, sizeof(address), "STACK,%u", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst("9H", "LDA", address, "rA " SYMB_ASSIGN " result " SYMB_EQUIV " STACK[SP]")) return -1;
  if (emit_inst(NULL, DEC(REG_SP), "1", "SP " SYMB_ASSIGN " SP - 1")) return -1;

  // set SP " SYMB_ASSIGN " FP (dealloc locals)
  if (snprintf(address, sizeof(address), "0,%u", REG_FP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, ENT(REG_SP), address, "SP " SYMB_ASSIGN " FP")) return -1;

  // pop FP
  if (snprintf(address, sizeof(address), "STACK,%u(0:2)", REG_FP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, LD(REG_FP), address, "FP " SYMB_ASSIGN " old FP " SYMB_EQUIV " STACK[SP]")) return -1;

  // pop RA
  if (snprintf(address, sizeof(address), "STACK-1,%u(0:2)", REG_SP)
      >= (int)sizeof(address)) return -1;
  if (emit_inst(NULL, "LD4", address, "rI4 " SYMB_ASSIGN " RA " SYMB_EQUIV " STACK[SP-1]")) return -1;

  // deallocate stack space for n_params
  if (snprintf(address, sizeof(address), "%u", n_params + 2)
      >= (int)sizeof(address)) return -1;
  if (snprintf(comment, sizeof(comment), "SP " SYMB_ASSIGN " SP - %u", n_params + 2)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, DEC(REG_SP), address, comment)) return -1;

  // push result to stack
  if (gen_push_reg('A')) return -1;

  // return to RA
  if (emit_inst(NULL, "JMP", "0,4", "jump to RA")) return -1;

  return 0;
}

int gen_method_return() {
  emit_comment("Return from subroutine (return value is top of the stack)");
  if (emit_inst(NULL, "JMP", "9F", "jump to method exit")) return -1;
  return 0;
}

int gen_method_call(const char *method_name, const char *label) {
  char comment[64];

  emit_comment("Call method %s (pop params, push result)", method_name);

  if (snprintf(comment, sizeof(comment), "jump to %s " SYMB_EQUIV " %s", label, method_name)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "JMP", label, comment)) return -1;

  return 0;
}

int gen_program_prologue(const char *entry_label, const char *main_label, unsigned int origin) {
  char address[32];
  char comment[64];

  if (snprintf(address, sizeof(address), "%d", origin)
      >= (int)sizeof(address)) return -1;

  emit_comment("Constants and memory locations");
  if (emit_inst("TTY", "EQU", "19", NULL)) return -1;
  if (emit_inst("BUFFER", "EQU", "3800", NULL)) return -1;
  if (emit_inst("STACK", "EQU", address, NULL)) return -1;

  emit_comment("Program entry, initializes SP, FP and jumps to main");

  // set origin address
  if (emit_inst(NULL, "ORIG", address, NULL)) return -1;

  // initialize SP and FP
  if (emit_inst(entry_label, ENT(REG_FP), "-STACK", "FP " SYMB_ASSIGN " 0")) return -1;
  if (emit_inst(NULL,        ENT(REG_SP), "-STACK", "SP " SYMB_ASSIGN " 0")) return -1;

  // jump to main
  if (snprintf(comment, sizeof(comment), "jump to main " SYMB_EQUIV " %s", main_label)
      >= (int)sizeof(comment)) return -1;
  if (emit_inst(NULL, "JMP", main_label, comment)) return -1;

  emit_comment("Pop result from stack and print");

  // pop return value 
  if (gen_pop_reg('A')) return -1;

  // convert to char and store in buffer
  if (emit_inst(NULL, "CHAR", NULL, NULL)) return -1;
  if (emit_inst(NULL, "STA", "BUFFER+7", "high byte of result")) return -1;
  if (emit_inst(NULL, "STX", "BUFFER+8", "low byte of result")) return -1;

  // print to TTY
  if (emit_inst(NULL, "OUT", "BUFFER(TTY)", "print to TTY")) return -1;
  if (emit_inst(NULL, "JBUS", "*(TTY)", "wait until printed")) return -1;

  emit_comment("Halt execution");

  // halt execution
  if (emit_inst(NULL, "HLT", NULL, NULL)) return -1;

  return 0;
}

int gen_program_epilogue(const char *entry_label) {
  emit_comment("Initial contents of buffer");
  if (emit_inst(NULL, "ORIG", "BUFFER", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"RETUR\"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"N VAL\"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"UE OF\"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\" MAIN\"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\" FUNC\"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"TION:\"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;
  if (emit_inst(NULL, "ALF", "\"     \"", NULL)) return -1;

  emit_comment("Program end, begin execution at %s", entry_label);
  if (emit_inst(NULL, "END", entry_label, NULL)) return -1;

  return 0;
}
