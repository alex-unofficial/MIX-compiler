#include "label.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static char *label_fmt(char prefix, unsigned int index) {

  const size_t buf_len = LABEL_LEN + 1;
  const unsigned int hex_digits = LABEL_LEN - 1;

  char *l = malloc(buf_len);
  if (!l) return NULL;

  uint64_t mask = (hex_digits >= 16)? UINT64_MAX : ((uint64_t) 1 << (4 * hex_digits)) - 1;

  snprintf(l, buf_len, "%c%0*llX", prefix, (int)hex_digits, (unsigned long long)(index & mask));

  return l;
}

char *label_method() {
  static unsigned int method_index = 1;
  return label_fmt('F', method_index++);
}

