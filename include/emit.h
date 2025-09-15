#ifndef EMIT_H
#define EMIT_H

#include <stdio.h>

extern FILE *mixout;

int emit_line(const char *fmt, ...);

#endif
