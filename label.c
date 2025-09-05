#include "label.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char *gen_method_label() {
  static unsigned int n = 1;

  const size_t buf_len = LABEL_LEN + 1;
  const unsigned int hex_digits = LABEL_LEN - 1;

  char *l = malloc(buf_len * sizeof(char));
  if (!l) return NULL;

  uint64_t mask;
  if (hex_digits >= sizeof(mask) * 8 / 4) {
    mask = UINT64_MAX;
  } else {
    mask = ((uint64_t) 1 << (4 * hex_digits)) - 1;
  }

  snprintf(l, buf_len, "F%0*llX", (int)hex_digits, (unsigned long long)((n++) & mask));

  return l;
}

