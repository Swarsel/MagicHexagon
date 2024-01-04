#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHANGES_LIMIT 6
#define NO_SOLUTION 0
#define CHANGE 1
#define NO_CHANGE 2

unsigned long num_rows;
unsigned long num_values;
long required_sum;
long offset; /* offset in occupation array */
unsigned long corners[6];
unsigned long* labelingIndices;


typedef struct var Entry;

/* constraint variable; if lo==hi, this is the variable's value */
typedef struct var {
  long id; /* variable id; id<0 if the variable is not part of the hexagon */
  long lower_bound; /* lower bound */
  long upper_bound; /* upper bound */
} Entry;

/* representation of a hexagon of order n: (2n-1)^2 square array
   for a hexagon of order 2:
     A B
    C D E
     F G
   the representation is:
    A B .
    C D E
    . F G
   The . slots have lo>hi.

   For order 3:
     A B C
    D E F G
   H I J K L
    M N O P
     Q R S
   the representation is:
    A B C . .
    D E F G .
    H I J K L
    . M N O P
    . . Q R S

   The variable array is organized as a single-dimension array with accesses
   vs[y*r+x]
   This allows to access the diagonal with stride 2*order

   Variable names n, r, H, S according to German Wikipedia Article
   Instead of "i", the deviation variable is called "d" (d=0 means
   that the sum=0; to have the lowest value 1, d=2)

   n is the order (number of elements of a side of the hexagon).
   r = 2n-1 (length of the middle row/diagonals)
   H = 3n^2-3n+1 (number of variables)
   M = dH (sum of each row/diagonal)
   lowest  value = dr - (H-1)/2
   highest value = dr + (H-1)/2
*/

unsigned long solutions = 0; /* counter of solutions */
unsigned long leafs = 0; /* counter of leaf nodes visited in the search tree */

#define swap(a,b,T) {T t = a; a = b; b = t;}

long min(long a, long b)
{
  return (a<b)?a:b;
}

long max(long a, long b)
{
  return (a>b)?a:b;
}

/* unless otherwise noted, the following functions return

   0 if there is no solution (i.e., the action eliminates all values
   from a variable),

   1 if there was a change

   2 if there was no change
*/


inline int sethigh(Entry *var, long new_value) {
  assert(var->id >= 0);
  if (new_value >= var->upper_bound) {
    return NO_CHANGE;
  }
  var->upper_bound = new_value;
  return (var->lower_bound <= var->upper_bound) ? CHANGE : NO_SOLUTION;
}

inline int setlow(Entry *var, long new_value) {
  assert(var->id >= 0);
  if (new_value <= var->lower_bound) {
    return NO_CHANGE;
  }
  var->lower_bound = new_value;
  return (var->lower_bound <= var->upper_bound) ? CHANGE : NO_SOLUTION;
}

/* returns 0 if there is no solution, 1 if one of the variables has changed */
int lessthan(Entry *entry1, Entry *entry2) {
  /* This is used between corner points to prevent symmetric solutions
     by effectively setting every corners range of allowed values to
     a value higher than that of the upper left corner, i.e. they are called
     with 'lessthan(&hexagon[corners[0]], &hexagon[corners[i]]);' */
  assert(entry1->id >= 0);
  assert(entry2->id >= 0);
  int res = sethigh(entry1, entry2->upper_bound - 1);
  // TODO: boolean check instead of res < NO_CHANGE
  if (res < NO_CHANGE)
    return res;
  /* if nothing has changed so far, set the other entries lower bound higher */
  return (setlow(entry2, entry1->lower_bound + 1));
}

int sum(Entry hexagon[], unsigned long num_elements, unsigned long stride,
        long sum, Entry *hexagon_start, Entry *hexagon_end) {
  /* computes new upper/lower bounds by first taking the sum that we are aiming
     for (M) and setting that to hi and lo. Then hi and low are reduced by the
       opposite value each since those values are used in the worst case and
       will as such never be used on the other side */
  unsigned long i;
  long hi = sum;
  long lo = sum;
  Entry *entry;
  /* #if 0 [...] #endif basically is a commented out block */
#if 0
  printf("sum(vsstart+%ld, %lu, %lu, %ld, vsstart, vsstart+%ld)   ",vs-vsstart,nv,stride,sum,vsend-vsstart); fflush(stdout);
  for (i=0, vp=vs; i<nv; i++, vp+=stride) {
    assert(vp>=vsstart);
    assert(vp<vsend);
    assert(vp->id >= 0);
    printf("v%ld ",vp->id);
  }
  printf("\n");
#endif
  for (i = 0, entry = hexagon; i < num_elements; i++, entry += stride) {
    assert(entry >= hexagon_start);
    assert(entry < hexagon_end);
    assert(entry->id >= 0);
    hi -= entry->lower_bound;
    lo -= entry->upper_bound;
  }
  /* hi is the upper bound of sum-sum(vs), lo the lower bound */
  for (i = 0, entry = hexagon; i < num_elements; i++, entry += stride) {
    /* readd vp->lo to get an upper bound of vp */
    int f = sethigh(entry, hi + entry->lower_bound);
    assert(entry >= hexagon_start);
    assert(entry < hexagon_end);
    assert(entry->id >= 0);
    if (f < NO_CHANGE)
      return f;
    f = setlow(entry, lo + entry->upper_bound); /* likewise, readd vp->hi */
    if (f < NO_CHANGE)
      return f;
  }
  return NO_CHANGE;
}

/* reduce the ranges of the variables as much as possible (with the
   constraints we use);  returns 1 if all variables still have a
   non-empty range left, 0 if one has an empty range */

  //printf("(re)start\n");
int solve(unsigned long side_length, long deviation, Entry hexagon[]) {
  /* num_rows = 2 * side_length - 1; */
  /* num_values = */
    /* 3 * side_length * side_length - 3 * side_length + 1; */
  /* the sum that is required in each row/column */
  /* required_sum = deviation * num_values; */
  /* offset in occupation array */
  /* offset = deviation * num_rows - (num_values - 1) / 2; */
  /* if hexagon[i] has value x, occupation[x-offset]==i, if no hexagon[*] has
   * value x, occupation[x-offset]==num_values */
  // => center the values so that smallest number is at idx=0
  unsigned long occupation[num_values];
  /* unsigned long corners[] = {0, */
  /*                            side_length - 1, */
  /*                            (side_length - 1) * num_rows + 0, */
  /*                            (side_length - 1) * num_rows + num_rows - 1, */
  /*                            (num_rows - 1) * num_rows + side_length - 1, */
  /*                            (num_rows - 1) * num_rows + num_rows - 1}; */
  unsigned long i;
  int changes_counter;
  /* deal with the alldifferent constraint */
  for (i = 0; i < num_values; i++)
    occupation[i] = num_rows * num_rows;
// TODO: Restart at i=i instead? Because lower/upper bound don't change in this
// loop, could then also remove the check for occupation != i
// -1 statt num_rows*num_rows
 restart:
  changes_counter = 0;
  /* use the alldifferent constraint */
  for (i = 0; i < num_rows * num_rows; i++) {
    Entry *entry = &hexagon[i];
    if (entry->lower_bound == entry->upper_bound &&
        occupation[entry->lower_bound - offset] != i) {
      if (occupation[entry->lower_bound - offset] < num_rows * num_rows)
        return 0; /* another variable has the same value */
      occupation[entry->lower_bound - offset] = i; /* occupy v->lo */
      changes_counter = 1;
    }
  }
  /* now propagate the alldifferent results to the bounds */
  for (i = 0; i < num_rows * num_rows; i++) {
    Entry *entry = &hexagon[i];
    if (entry->lower_bound < entry->upper_bound) {
      while (occupation[entry->lower_bound - offset] < num_rows * num_rows) {
        entry->lower_bound++;
        if(entry->lower_bound > entry->upper_bound)
          return 0;
        changes_counter = 1;
      }
      
      while (occupation[entry->upper_bound - offset] < num_rows * num_rows) {
        entry->upper_bound--;
        changes_counter = 1;
      }
      if(entry->lower_bound > entry->upper_bound)
        return 0;
    }
  }
  /* the < constraints; all other corners are smaller than the first
     one (eliminate rotational symmetry) */
  // TODO: define num_corners instead of doing sizeof/sizeof => no division
  for (i = 1; i < sizeof(corners) / sizeof(corners[0]); i++) {
    int f = lessthan(&hexagon[corners[0]], &hexagon[corners[i]]);
    if (f == NO_SOLUTION)
      return NO_SOLUTION;
    if (f == CHANGE)
      changes_counter = 1;
  }
  /* eliminate the mirror symmetry between the corners to the right
     and left of the first corner */
  {
    int f = lessthan(&hexagon[corners[2]], &hexagon[corners[1]]);
    if (f == NO_SOLUTION)
      return NO_SOLUTION;
    if (f == CHANGE)
      changes_counter = 1;
  }
  /* sum constraints: each line and diagonal sums up to M */
  /* line sum constraints */
  for (i = 0; i < num_rows; i++) {
    int f;
    /* line */
    f = sum(hexagon + num_rows * i + max(0, i + 1 - side_length),
            min(i + side_length, num_rows + side_length - i - 1), 1,
            required_sum, hexagon, hexagon + num_rows * num_rows);
    if (f == NO_SOLUTION)
      return NO_SOLUTION;
    if (f == CHANGE)
      changes_counter = 1;
    /* column (diagonal down-left in the hexagon) */
    f = sum(hexagon + i + max(0, i + 1 - side_length) * num_rows,
            min(i + side_length, num_rows + side_length - i - 1), num_rows,
            required_sum, hexagon, hexagon + num_rows * num_rows);
    if (f == NO_SOLUTION)
      return NO_SOLUTION;
    if (f == CHANGE)
      changes_counter = 1;
    /* diagonal (down-right) */
    f = sum(hexagon - side_length + 1 + i +
                max(0, side_length - i - 1) * (num_rows + 1),
            min(i + side_length, num_rows + side_length - i - 1), num_rows + 1,
            required_sum, hexagon, hexagon + num_rows * num_rows);
    if (f == NO_SOLUTION)
      return NO_SOLUTION;
    if (f == CHANGE)
      changes_counter = 1;
  }
  if (changes_counter > 0) goto restart;
  return 1;
}

void printhexagon(unsigned long side_length, Entry hexagon[]) {
  unsigned long i, j;
  for (i = 0; i < num_rows; i++) {
    unsigned long l = 0;
    unsigned long h = num_rows;
    if (i + 1 > side_length)
      l = i + 1 - side_length;
    if (i + 1 < side_length)
      h = side_length + i;
    for (j = h - l; j < num_rows; j++)
      printf("    ");
    for (j = l; j < h; j++) {
      assert(i < num_rows);
      assert(j < num_rows);
      Entry *v = &hexagon[i * num_rows + j];
      assert(v->lower_bound <= v->upper_bound);
#if 0
      printf("%6ld  ",v->id);
#else
      if (v->lower_bound < v->upper_bound)
        printf("%4ld-%-3ld", v->lower_bound, v->upper_bound);
      else
        printf("%6ld  ", v->lower_bound);
#endif
    }
    printf("\n");
  }
}

int heuristic(Entry hexagon[], int minLabelIndex, unsigned long * labelingIndices){
  int index = minLabelIndex;
  long max = 0;
  for (int i=minLabelIndex;i<num_rows*num_rows;i++){
    Entry *entry = &hexagon[labelingIndices[i]];
    if(entry->id > -1){
      long lower_bound = entry -> lower_bound; 
      if(lower_bound > max && lower_bound >= 0){
      max = lower_bound;
      index = i;
    }
  }
}
  return index;
}
/* assign values to hexagon[index] and all later variables in hexagon such that
   the constraints hold */
void labeling(unsigned long side_length, long deviation, Entry hexagon[],
              unsigned long* labelingIndices, long index) {
  Entry *entry = &hexagon[labelingIndices[index]];
  /* because our representation yields row * row entries, if an entry has
     survived up to that index, it must be a solution */
  if (index >= num_rows * num_rows) {
    printhexagon(side_length, hexagon);
    solutions++;
    leafs++;
    printf("leafs visited: %lu\n\n", leafs);
    return;
  }
  
if (entry->id < 0){
    /* this skips the entries that are not part of the hexagon '.' */
    return labeling(side_length, deviation, hexagon, labelingIndices, index + 1);
  }

  if(entry->lower_bound >= entry->upper_bound){
    unsigned long nextIdx = heuristic(hexagon, index+1, labelingIndices);
    if (nextIdx == index+1){
      return labeling(side_length, deviation, hexagon, labelingIndices, index + 1);
    }
    else{
      swap(labelingIndices[index+1], labelingIndices[nextIdx], unsigned long);
      labeling(side_length, deviation, hexagon, labelingIndices, index + 1);
      swap(labelingIndices[nextIdx], labelingIndices[index + 1], unsigned long)
      return;
    }
  }

  long middle = (entry->lower_bound + entry->upper_bound)/2;
  if(entry->lower_bound + entry->upper_bound < 0 && (entry->lower_bound + entry->upper_bound) % 2 != 0){
    middle--;
  }

  /* for (i = entry->lower_bound; i <= entry->upper_bound; i++) { */
    /* make new variables that are to be tested for solution
       these are tested with a fixed value */
  Entry new_hexagon[num_rows * num_rows];
  Entry *new_entry = new_hexagon + labelingIndices[index];
  memmove(new_hexagon, hexagon, num_rows * num_rows * sizeof(Entry));
  new_entry->lower_bound = entry->lower_bound;
  new_entry->upper_bound = middle;
  if (solve(side_length, deviation, new_hexagon))
    labeling(side_length, deviation, new_hexagon, labelingIndices, index);
  else {
    leafs++;
  }
  memmove(new_hexagon,hexagon,num_rows*num_rows*sizeof(Entry));
  new_entry->lower_bound = middle+1;
  new_entry->upper_bound = entry->upper_bound;
  if (solve(side_length,deviation,new_hexagon)){
    labeling(side_length,deviation,new_hexagon, labelingIndices, index);
  }
  else{
    leafs++;
  }
}

Entry *makehexagon(unsigned long side_length, long deviation) {
  unsigned long i, j;

  Entry *hexagon = calloc(num_rows * num_rows, sizeof(Entry));
  unsigned long id = 0;
  for (i = 0; i < num_rows * num_rows; i++) {
    Entry *v = &hexagon[i];
    v->id = -1;
    v->lower_bound = 1;
    v->upper_bound = 0;
  }
  for (i = 0; i < num_rows; i++) {
    //   n = 3
    //   A B C     i=0, start=0, end=3
    //  D E F G    i=1, start=0, end=4
    // H I J K L   i=2, start=0, end=5
    //  M N O P    i=3, start=1, end=num_rows
    //   Q R S     i=4, start=2, end=num_rows
    unsigned long start = 0;
    unsigned long end = num_rows;
    if (i + 1 > side_length)
      start = i + 1 - side_length;
    if (i + 1 < side_length)
      end = side_length + i;
    for (j = start; j < end; j++) {
      assert(i < num_rows);
      assert(j < num_rows);
      Entry *v = &hexagon[i * num_rows + j];
      assert(v->lower_bound > v->upper_bound);
      v->id = id++;
      v->lower_bound = deviation * num_rows - (num_values - 1) / 2;
      v->upper_bound = deviation * num_rows + (num_values - 1) / 2;
    }
  }
  return hexagon;
}

int main(int argc, char *argv[]) {
  unsigned long i;
  unsigned long j = 0;
  long deviation;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <side_length> <deviation> <value> ... <value>\n",
            argv[0]);
    exit(1);
  }
  const unsigned long side_length = strtoul(argv[1], NULL, 10);
  num_rows = 2*side_length -1;
  num_values = 3*side_length*side_length-3*side_length+1;
  if (side_length < 1) {
    fprintf(stderr, "order must be >=1\n");
    exit(1);
  }
  deviation = strtol(argv[2], NULL, 10);
  required_sum = deviation*num_values;
  offset = deviation*num_rows - (num_values-1)/2; /* offset in occupation array */
  Entry *hexagon = makehexagon(side_length, deviation);
  for (i = 3; i < argc; i++) {
    while (hexagon[j].id < 0)
      j++;
    hexagon[j].lower_bound = hexagon[j].upper_bound = strtol(argv[i], NULL, 10);
    j++;
  }
  corners[0] = 0;
  corners[1] = side_length-1;
  corners[2] = (side_length-1)*num_rows+0;
  corners[3] = (side_length-1)*num_rows+num_rows-1;
  corners[4] = (num_rows-1)*num_rows+side_length-1;
  corners[5] = (num_rows-1)*num_rows+num_rows-1;
  labelingIndices = malloc(num_rows*num_rows*sizeof(unsigned long));
  for(i=0; i<num_rows*num_rows; i++){
    labelingIndices[i] = i;
  }
  for(i=0;i<6;i++){
    swap(labelingIndices[i],labelingIndices[corners[i]], unsigned long);
  }

  labeling(side_length, deviation, hexagon, labelingIndices, 0);
  printf("%lu solution(s), %lu leafs visited\n", solutions, leafs);
  //(void)solve(n, d, vs);
  // printhexagon(n, vs);
  free(labelingIndices);
  return 0;
}
