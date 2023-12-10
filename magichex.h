#ifndef __MAGICHEX_H__
#define __MAGICHEX_H__
typedef struct var Var;

/* constraint variable; if lo==hi, this is the variable's value */
typedef struct var {
  long id; /* variable id; id<0 if the variable is not part of the hexagon */
  long lo; /* lower bound */
  long hi; /* upper bound */
  unsigned long lorank;
  unsigned long hirank;
} Var;

int sethi(Var *v, long x);
int setlo(Var *v, long x);

#endif