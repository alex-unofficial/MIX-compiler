#include "label.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *label_fmt(const char *prefix, unsigned int index) {

  const size_t buf_len = LABEL_LEN + 1;
  const int hex_digits = LABEL_LEN - strlen(prefix);
  if (hex_digits <= 0) return NULL;

  char *l = malloc(buf_len);
  if (!l) return NULL;

  uint64_t mask = (hex_digits >= 16)? UINT64_MAX : ((uint64_t) 1 << (4 * hex_digits)) - 1;

  if(snprintf(l, buf_len, "%s%0*llX", prefix, 
        (int)hex_digits, (unsigned long long)(index & mask)) 
     >= (int)buf_len) {
    free(l);
    return NULL;
  }

  return l;
}

char *label_method(unsigned int index) {
  return label_fmt("FUNC", index);
}

char *label_loop(unsigned int index) {
  return label_fmt("LOOP", index);
}

char *label_else(unsigned int index) {
  return label_fmt("ELSE", index);
}

char *label_done(unsigned int index) {
  return label_fmt("DONE", index);
}
