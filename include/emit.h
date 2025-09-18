#ifndef EMIT_H
#define EMIT_H

#include <stdio.h>

#define COLWIDTH 80

extern FILE *mixout;

int emit_line(const char *fmt, ...);
int emit_comment(const char *fmt, ...);
int emit_label(const char *label);

int emit_inst(const char *label,
              const char *opcode,
              const char *addexp,
              const char *comment);

#endif
