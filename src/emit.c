#include "emit.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define COL_LABEL 10
#define COL_OPCODE 4
#define COL_ADDR 22

FILE *mixout = NULL;

int emit_line(const char *fmt, ...) {
  if (!mixout) return -1;

  va_list args;

  va_start(args, fmt);
  vfprintf(mixout, fmt, args);
  va_end(args);

  fputc('\n', mixout);
  return 0;
}

int emit_comment(const char *fmt, ...) {
  if (!mixout) return -1;

  va_list args;

  fprintf(mixout, "* ");
  va_start(args, fmt);
  vfprintf(mixout, fmt, args);
  va_end(args);
  fputc('\n', mixout);

  return 0;
}

int emit_label(const char *label) {
  return emit_inst(label, "NOP", NULL, NULL);
}

int emit_inst(const char *label,
              const char *opcode,
              const char *address,
              const char *comment) {

  if (!mixout) return -1;

  fprintf(mixout, "%-*s", COL_LABEL, label ? label : "");
  fprintf(mixout, " %-*s", COL_OPCODE, opcode);
  fprintf(mixout, " %-*s", COL_ADDR, address ? address : "");
  if (comment && comment[0]) {
    fprintf(mixout, " : %s", comment);
  }
  fputc('\n', mixout);

  return 0;
}
