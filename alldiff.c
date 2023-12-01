#include "alldiff.h"
#include "magichex.h"

long pathmax(long a[], long x) {
    while (a[x] > x)
      x = a[x];
    return x;
}

long pathmax_record(long a[], long x, long* record[], long* n) {
  *n = 0;
  while (a[x] > x){
    record[*n] = &a[x];
    (*n)++;
    x = a[x];
  }
  return x;
}

long pathmin(long v[], long i) {
  while (v[i] < i)
      i = v[i];
  return i;
}
long pathmin_record(long v[], long i,long* record[], long* n) {
  *n = 0;
  while (v[i] < i){
      record[*n] = &v[i];
      (*n)++;
      i = v[i];
  }
  return i;
}
void pathset(long v[], long start, long end, long to) {
    long next = start;
    long prev = next;
    while (prev != end) {
        next = v[prev];
        v[prev] = to;
        prev = next;
    }
}
void pathset_record(long* record[], long n, long to) {
    for(long i = 0; i < n; i++){
      *record[i] = to;
    }
}



void insertion_sort_lo(TYPE A[], int n) {
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

void insertion_sort_hi(TYPE A[], int n) {
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

void counting_sort(TYPE a[], int length, int min, int max){
	/*for a[] = {0, 3, 2, 3, 3, 0, 5, 2, 3} range will be	0, ... , 5. 
		So we will need 6 spots in our new sub-array which is max + 1*/
	int i;
  int len = max-min;
	long c[(len)+1];
	for(i = 0; i < len + 1; c[i++] = 0); // first c[i] = 0 will be done, then i++;
	for(i = 0; i < length; i++) c[a[i]->lo-min]++;
	for(i = 1; i < len + 1; i++) c[i] = c[i - 1] + c[i];
	TYPE b[length]; //this is gonna be our new sorted array
	int j = length - 1;
	for(i = 0; i < length; i++) {
		b[--c[a[j]->lo-min]] = a[j];
		j--;
	}	
	for(i = 0; i < length; i++)	a[i] = b[i];
}

void counting_sort_hi(TYPE a[], int length, int min, int max){
	int i;
  int len = max-min;
	long c[(len)+1];
	
	for(i = 0; i < len + 1; c[i++] = 0); 
	for(i = 0; i < length; i++) c[a[i]->hi-min]++;
	for(i = 1; i < len + 1; i++) c[i] = c[i - 1] + c[i];
	TYPE b[length]; 
	int j = length - 1;
	for(i = 0; i < length; i++) {
		b[--c[a[j]->hi-min]] = a[j];
		j--;
	}	
	for(i = 0; i < length; i++)	a[i] = b[i];
}

int alldifferent(Var vs[], Var* minsorted[],Var* maxsorted[],long size, long minVal, long maxVal, char* partSorted){
  int runningIndex = 0;
  int f = 2;
  runningIndex = size;

  if(*partSorted == 0){
    counting_sort_hi(maxsorted, runningIndex,minVal, maxVal);
    counting_sort(minsorted, runningIndex, minVal, maxVal);
    *partSorted = 1;
  } else {
    insertion_sort_lo(minsorted, runningIndex);
    insertion_sort_hi(maxsorted, runningIndex);
  }
  

  long bounds[2*runningIndex];
  long min = minsorted[0]->lo;
  long max = maxsorted[0]->hi+1;
  unsigned long nb = 0;
  long last = min - 2;
  bounds[nb] = last;
  unsigned long i = 0, j = 0;

  while(1){
    if(i < runningIndex && min <= max) {
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

  unsigned long niv = runningIndex;
  long nrec = 0;
  long t[2*runningIndex], d[2*runningIndex], h[2*runningIndex];
  long *record[2*runningIndex];

  //Lower Bounds
  for (int i = 1; i <= nb+1; i++) {
    t[i] = h[i] = i-1;
    d[i] = bounds[i] - bounds[i-1];
  }
  for (long i = 0; i < niv; i++) {
    unsigned long x = maxsorted[i]->lorank;
    unsigned long y = maxsorted[i]->hirank;
    long z = pathmax_record(t, x+1, record, &nrec);
    long j = t[z];
    if(--d[z] == 0){
      t[z]= z+1;
      z = pathmax(t, t[z]);
      t[z] = j;
    }
    pathset_record(record, nrec, z);
    
    if(d[z] < bounds[z]-bounds[y]){
      return 0;
    }
    if (h[x] > x){
      long w = pathmax_record(h, h[x],record,&nrec);
      maxsorted[i]->lo = bounds[w];
      f = 1;
      pathset_record(record,nrec,w);
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
    long z = pathmin_record(t, x-1,record,&nrec);
    long j = t[z];
    if(--d[z] == 0){
      t[z]= z-1;
      z = pathmin(t, t[z]);
      t[z] = j;
    }
    pathset_record(record, nrec, z);
    if(d[z] < bounds[y]-bounds[z]){
      return 0;
    }
    if (h[x] < x){
      long w = pathmin_record(h, h[x],record,&nrec);
      minsorted[i]->hi = bounds[w]-1;
      f = 1;
      pathset_record(record, nrec, w);
      //pathset(h, x, w, w);
    }
    if(d[z] == bounds[y]-bounds[z]){
      pathset(h, h[y], j+1, y);
      h[y] = j+1;
    }
  }
  return f;
}


