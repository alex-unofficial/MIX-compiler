#include "label.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static char *gen_label(char prefix, unsigned int index) {

  const size_t buf_len = LABEL_LEN + 1;
  const unsigned int hex_digits = LABEL_LEN - 1;

  char *l = malloc(buf_len);
  if (!l) return NULL;

  uint64_t mask = (hex_digits >= 16)? UINT64_MAX : ((uint64_t) 1 << (4 * hex_digits)) - 1;

  snprintf(l, buf_len, "%c%0*llX", prefix, (int)hex_digits, (unsigned long long)(index & mask));

  return l;
}

char *gen_method_label() {
  static unsigned int method_index = 1;
  return gen_label('F', method_index++);
}

char *gen_branch_label() {
  static unsigned int branch_index = 1;
  return gen_label('L', branch_index++);
}
