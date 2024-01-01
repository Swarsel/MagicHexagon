#ifndef __MAGICHEX_H__
#define __MAGICHEX_H__

/* constraint variable; if lo==hi, this is the variable's value */
typedef struct var {
  long id; /* variable id; id<0 if the variable is not part of the hexagon */
  long lower_bound; /* lower bound */
  long upper_bound; /* upper bound */
  unsigned long lorank;
  unsigned long hirank;
} Entry;

int sethigh(Entry *v, long x);
int setlow(Entry *v, long x);

#endif