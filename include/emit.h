#ifndef EMIT_H
#define EMIT_H

#include <stdio.h>

extern FILE *mixout;

int emit_line(const char *fmt, ...);
int emit_comment(const char *fmt, ...);
int emit_label(const char *label);

int emit_inst(const char *label,
              const char *opcode,
              const char *address,
              const char *comment);

#endif
