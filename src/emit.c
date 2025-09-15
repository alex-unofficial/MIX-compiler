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
