#include "emit.h"

#include <stdio.h>
#include <stdarg.h>

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

int emit_inst(const char *label,
              const char *opcode,
              const char *address,
              const char *comment) {

  if (!mixout) return -1;

  fprintf("%-10s", label ? label : "");
  fprintf(" %-4s", opcode);
  fprintf(" %-22s", address ? address : "");
  if (comment && comment[0]) {
    fprintf(" ; %s", comment);
  }
  fputc('\n', mixout);

  return 0;
}

