#ifndef _ALLDIFF_H_
#define _ALLDIFF_H_

#include "magichex.h"

typedef Entry* TYPE;
#define swap(t, x, y) { t z = x; x = y; y = z; }

int alldifferent(Entry vs[], Entry* minsorted[],Entry* maxsorted[],long size, long minVal, long maxVal, char* partSorted);

#endif