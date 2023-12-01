#ifndef _ALLDIFF_H_
#define _ALLDIFF_H_

#include "magichex.h"

typedef Var* TYPE;
#define swap(t, x, y) { t z = x; x = y; y = z; }

int alldifferent(Var vs[], Var* minsorted[],Var* maxsorted[],long size, long minVal, long maxVal, char* partSorted);

#endif