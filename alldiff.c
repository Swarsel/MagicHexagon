#include "alldiff.h"
#include "magichex.h"
#include <stdlib.h>
#include <string.h>

inline unsigned char pathmax(unsigned char a[], unsigned char x) {
  
    while (a[x] > x)
      x = a[x];
    return x;
}

inline unsigned char pathmax_record(unsigned char a[], unsigned char x, unsigned char* record[], long* n) {
  long ln = 0;
  while (a[x] > x){
    record[ln++] = a+x;
    x = a[x];
  }
  *n = ln;
  return x;
}

inline unsigned char pathmin(unsigned char v[], unsigned char i) {
  
  while (v[i] < i)
      i = v[i];
  return i;
}

inline unsigned char pathmin_record(unsigned char v[], unsigned char i,unsigned char* record[], long* n) {
  long ln = 0;
  while (v[i] < i){
      record[ln++] = &v[i];
      i = v[i];
  }
  *n = ln;
  return i;
}

inline void pathset(unsigned char v[], unsigned char start, unsigned char end, unsigned char to) {
    unsigned char next = start;
    unsigned char prev = next;
    while (prev != end) {
        next = v[prev];
        v[prev] = to;
        prev = next;
    }
}

inline void pathset_record(unsigned char* record[], unsigned char n, unsigned char to) {
    for(long i = 0; i < n; i++){
      *record[i] = to;
    }
}

inline void insertion_sort_lo(TYPE A[], int n) {
	int i, j;
	TYPE temp;
	for(i = 1; i < n; i++) {
		temp = A[i];
		j = i;
		while(j > 0 && A[j-1]->lo > temp->lo) {
			A[j] = A[j - 1];
			j--;
		}
		A[j] = temp;
	}
}

inline void insertion_sort_hi(TYPE A[], int n) {
	int i, j;
	TYPE temp;
	for(i = 1; i < n; i++) {
		temp = A[i];
		j = i;
		while(j > 0 && A[j-1]->hi > temp->hi) {
			A[j] = A[j - 1];
			j--;
		}
		A[j] = temp;
	}
}

inline void counting_sort(TYPE a[], TYPE b[], int length, int min, int max){
	/*for a[] = {0, 3, 2, 3, 3, 0, 5, 2, 3} range will be	0, ... , 5. 
		So we will need 6 spots in our new sub-array which is max + 1*/
	int i;
  int len = max-min;
	long c[(len)+1];
  memset(c,0,(len+1)*sizeof(long));
	for(i = 0; i < length; i++) c[a[i]->lo-min]++;
	for(i = 1; i < len + 1; i++) c[i] = c[i - 1] + c[i];
	for(i = length - 1; i >= 0; i--) {
        b[--c[a[i]->lo-min]] = a[i];
  }	

}

inline void counting_sort_hi(TYPE a[], TYPE b[], int length, int min, int max){
	int i;
  int len = max-min;
	long c[(len)+1];
	memset(c,0,(len+1)*sizeof(long));
	for(i = 0; i < length; i++) c[a[i]->hi-min]++;
	for(i = 1; i < len + 1; i++) c[i] = c[i - 1] + c[i];
	for(i = length - 1; i >= 0; i--) {
        b[--c[a[i]->hi-min]] = a[i];
  }

}

unsigned char *t, *d, *h;
long *bounds;

int alldifferent(Var vs[], Var* minsorted[],Var* maxsorted[],long size, long minVal, long maxVal, char* partSorted){
  static char firstTime = 1;
  int runningIndex = size;
  long lenBounds = 2*runningIndex;
  if(firstTime)
  {
    t = malloc(lenBounds*sizeof(unsigned char));
    d = malloc(lenBounds*sizeof(unsigned char));
    h = malloc(lenBounds*sizeof(unsigned char));
    bounds = malloc(lenBounds*sizeof(long));
    firstTime = 0;
  }
  int f = 2;
  unsigned long niv = runningIndex;
  
  if(!(*partSorted))
  {
    counting_sort_hi(minsorted,maxsorted,runningIndex,minVal, maxVal);
    counting_sort(maxsorted,minsorted,runningIndex, minVal, maxVal);
    *partSorted = 1;
  } else {
    insertion_sort_hi(maxsorted,runningIndex);
    insertion_sort_lo(minsorted,runningIndex);
  }

  long min = minsorted[0]->lo;
  long max = maxsorted[0]->hi+1;
  unsigned long nb = 0;
  long last = min - 2;
  bounds[nb] = last;
  unsigned long i = 0, j = 0;

  while(1){
    if(min <= max && i < runningIndex) {
      if(min != last)
        bounds[++nb] = last = min;
      minsorted[i]->lorank = nb;
      if(++i < runningIndex)
        min = minsorted[i]->lo;
    } else {
      if(max != last)
        bounds[++nb] = last = max;
      maxsorted[j]->hirank = nb;
      if(++j == runningIndex)
        break;
      max = maxsorted[j]->hi+1;
    }
  }
  bounds[nb+1] = bounds[nb]+2;

  
  //Lower Bounds
  for (int i = 1; i <= nb+1; i++) {
    t[i] = h[i] = i-1;
    d[i] = bounds[i] - bounds[i-1];
  }
  for (long i = 0; i < niv; i++) {
    unsigned long x = maxsorted[i]->lorank;
    unsigned long y = maxsorted[i]->hirank;
    long z = pathmax(t, x+1);
    long j = t[z];
    if(--d[z] == 0){
      t[z]= z+1;
      z = pathmax(t, t[z]);
      t[z] = j;
    }
    pathset(t,x+1,z,z);
    
    if(d[z] < bounds[z]-bounds[y]){
      return 0;
    }
    if (h[x] > x){
      long w = pathmax(h, h[x]);
      f = setlo(maxsorted[i], bounds[w]);
      if(f == 0)
        return 0;
      pathset(h, x, w, w);
    }
    if(d[z] == bounds[z]-bounds[y]){
      pathset(h, h[y], j-1, y);
      h[y] = j-1;
    }
  }
  //Upper Bounds
  for (int i = 0; i <= nb; i++) {
    t[i] = h[i] = i+1;
    d[i] = bounds[i+1] - bounds[i];
  }
  for (long i = niv-1; i >=0 ; i--) {
    unsigned long x = minsorted[i]->hirank;
    unsigned long y = minsorted[i]->lorank;
    long z = pathmin(t, x-1);
    long j = t[z];
    if(--d[z] == 0){
      t[z]= z-1;
      z = pathmin(t, t[z]);
      t[z] = j;
    }
    //pathset_record(record, nrec, z);
    pathset(t,x-1,z,z);
    if(d[z] < bounds[y]-bounds[z]){
      return 0;
    }
    if (h[x] < x){
      long w = pathmin(h, h[x]);
      f = sethi(minsorted[i], bounds[w]-1);
      if(f == 0)
        return 0;
      pathset(h, x, w, w);
    }
    if(d[z] == bounds[y]-bounds[z]){
      
      pathset(h, h[y], j+1, y);
      h[y] = j+1;
    }
  }
  return f;
}


