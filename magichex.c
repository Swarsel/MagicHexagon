#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

long min(long a, long b) { return (a < b) ? a : b; }

long max(long a, long b) { return (a > b) ? a : b; }

/* unless otherwise noted, the following functions return

   0 if there is no solution (i.e., the action eliminates all values
   from a variable),

   1 if there was a change

   2 if there was no change
*/

enum RESULT { NOSOLUTION = 0, CHANGE = 1, NOCHANGE = 2 };

int sethigh(Entry *var, long new_value) {
  assert(var->id >= 0);
  if (new_value < var->upper_bound) {
    var->upper_bound = new_value;
    if (var->lower_bound <= var->upper_bound)
      return CHANGE;
    else
      return NOSOLUTION;
  }
  return NOCHANGE;
}

int setlow(Entry *var, long new_value) {
  assert(var->id >= 0);
  if (new_value > var->lower_bound) {
    var->lower_bound = new_value;
    if (var->lower_bound <= var->upper_bound)
      return CHANGE;
    else
      return NOSOLUTION;
  }
  return NOCHANGE;
}

/* returns 0 if there is no solution, 1 if one of the variables has changed */
int lessthan(Entry *entry1, Entry *entry2) {
  assert(entry1->id >= 0);
  assert(entry2->id >= 0);
  int res = sethigh(entry1, entry2->upper_bound - 1);
  // TODO: boolean check instead of res < NOCHANGE
  if (res < NOCHANGE)
    return res;
  return (setlow(entry2, entry1->lower_bound + 1));
}

int sum(Entry hexagon[], unsigned long num_elements, unsigned long stride,
        long sum, Entry *hexagonstart, Entry *hexagonend) {
  unsigned long i;
  long hi = sum;
  long lo = sum;
  Entry *entry;
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
    assert(entry >= hexagonstart);
    assert(entry < hexagonend);
    assert(entry->id >= 0);
    hi -= entry->lower_bound;
    lo -= entry->upper_bound;
  }
  /* hi is the upper bound of sum-sum(vs), lo the lower bound */
  for (i = 0, entry = hexagon; i < num_elements; i++, entry += stride) {
    /* readd vp->lo to get an upper bound of vp */
    int f = sethigh(entry, hi + entry->lower_bound);
    assert(entry >= hexagonstart);
    assert(entry < hexagonend);
    assert(entry->id >= 0);
    if (f < NOCHANGE)
      return f;
    f = setlow(entry, lo + entry->upper_bound); /* likewise, readd vp->hi */
    if (f < NOCHANGE)
      return f;
  }
  return NOCHANGE;
}

/* reduce the ranges of the variables as much as possible (with the
   constraints we use);  returns 1 if all variables still have a
   non-empty range left, 0 if one has an empty range */
int solve(unsigned long side_length, long deviation, Entry hexagon[]) {
  unsigned long num_rows = 2 * side_length - 1;
  unsigned long num_values =
      3 * side_length * side_length - 3 * side_length + 1;
  long MagicNumber = deviation * num_values;
  /* offset in occupation array */
  long offset = deviation * num_rows - (num_values - 1) / 2;
  /* if hexagon[i] has value x, occupation[x-offset]==i, if no hexagon[*] has
   * value x, occupation[x-offset]==num_values */
  // => center the values so that smallest number is at idx=0
  unsigned long occupation[num_values];
  unsigned long corners[] = {0,
                             side_length - 1,
                             (side_length - 1) * num_rows + 0,
                             (side_length - 1) * num_rows + num_rows - 1,
                             (num_rows - 1) * num_rows + side_length - 1,
                             (num_rows - 1) * num_rows + num_rows - 1};
  unsigned long i;
  /* deal with the alldifferent constraint */
  for (i = 0; i < num_values; i++)
    occupation[i] = num_rows * num_rows;
// TODO: Restart at i=i instead? Because lower/upper bound don't change in this
// loop, could then also remove the check for occupation != i
// -1 statt num_rows*num_rows
restart:
  for (i = 0; i < num_rows * num_rows; i++) {
    Entry *entry = &hexagon[i];
    if (entry->lower_bound == entry->upper_bound &&
        occupation[entry->lower_bound - offset] != i) {
      if (occupation[entry->lower_bound - offset] < num_rows * num_rows)
        return 0; /* another variable has the same value */
      occupation[entry->lower_bound - offset] = i; /* occupy v->lo */
      goto restart;
    }
  }
  /* now propagate the alldifferent results to the bounds */
  for (i = 0; i < num_rows * num_rows; i++) {
    Entry *entry = &hexagon[i];
    if (entry->lower_bound < entry->upper_bound) {
      if (occupation[entry->lower_bound - offset] < num_rows * num_rows) {
        entry->lower_bound++;
        goto restart;
      }
      if (occupation[entry->upper_bound - offset] < num_rows * num_rows) {
        entry->upper_bound--;
        goto restart;
      }
    }
  }
  /* the < constraints; all other corners are smaller than the first
     one (eliminate rotational symmetry) */
  // TODO: define num_corners instead of doing sizeof/sizeof => no division
  for (i = 1; i < sizeof(corners) / sizeof(corners[0]); i++) {
    int f = lessthan(&hexagon[corners[0]], &hexagon[corners[i]]);
    if (f == NOSOLUTION)
      return NOSOLUTION;
    if (f == CHANGE)
      goto restart;
  }
  /* eliminate the mirror symmetry between the corners to the right
     and left of the first corner */
  {
    int f = lessthan(&hexagon[corners[2]], &hexagon[corners[1]]);
    if (f == NOSOLUTION)
      return NOSOLUTION;
    if (f == CHANGE)
      goto restart;
  }
  /* sum constraints: each line and diagonal sums up to M */
  /* line sum constraints */
  for (i = 0; i < num_rows; i++) {
    int f;
    /* line */
    f = sum(hexagon + num_rows * i + max(0, i + 1 - side_length),
            min(i + side_length, num_rows + side_length - i - 1), 1,
            MagicNumber, hexagon, hexagon + num_rows * num_rows);
    if (f == NOSOLUTION)
      return NOSOLUTION;
    if (f == CHANGE)
      goto restart;
    /* column (diagonal down-left in the hexagon) */
    f = sum(hexagon + i + max(0, i + 1 - side_length) * num_rows,
            min(i + side_length, num_rows + side_length - i - 1), num_rows,
            MagicNumber, hexagon, hexagon + num_rows * num_rows);
    if (f == NOSOLUTION)
      return NOSOLUTION;
    if (f == CHANGE)
      goto restart;
    /* diagonal (down-right) */
    f = sum(hexagon - side_length + 1 + i +
                max(0, side_length - i - 1) * (num_rows + 1),
            min(i + side_length, num_rows + side_length - i - 1), num_rows + 1,
            MagicNumber, hexagon, hexagon + num_rows * num_rows);
    if (f == NOSOLUTION)
      return NOSOLUTION;
    if (f == CHANGE)
      goto restart;
  }
  return 1;
}

void printhexagon(unsigned long n, Entry vs[]) {
  unsigned long i, j;
  unsigned r = 2 * n - 1;
  for (i = 0; i < r; i++) {
    unsigned long l = 0;
    unsigned long h = r;
    if (i + 1 > n)
      l = i + 1 - n;
    if (i + 1 < n)
      h = n + i;
    for (j = h - l; j < r; j++)
      printf("    ");
    for (j = l; j < h; j++) {
      assert(i < r);
      assert(j < r);
      Entry *v = &vs[i * r + j];
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

/* assign values to hexagon[index] and all later variables in hexagon such that
   the constraints hold */
void labeling(unsigned long side_length, long deviation, Entry hexagon[],
              unsigned long index) {
  long i;
  unsigned long num_rows = 2 * side_length - 1;
  if (index >= num_rows * num_rows) {
    printhexagon(side_length, hexagon);
    solutions++;
    leafs++;
    printf("leafs visited: %lu\n\n", leafs);
    return;
  }
  Entry *entry = &hexagon[index];
  if (entry->id < 0)
    return labeling(side_length, deviation, hexagon, index + 1);
  for (i = entry->lower_bound; i <= entry->upper_bound; i++) {
    Entry newhexagon[num_rows * num_rows];
    Entry *newentry = &newhexagon[index];
    memmove(newhexagon, hexagon, num_rows * num_rows * sizeof(Entry));
    newentry->lower_bound = i;
    newentry->upper_bound = i;
#if 0
    for (Var *v = newvs; v<=newvp; v++) {
      if (v->id >= 0) {
        assert(v->lo == v->hi);
        printf(" %ld",v->lo); fflush(stdout);
      }
    }
    printf("\n");
#endif
    if (solve(side_length, deviation, newhexagon))
      labeling(side_length, deviation, newhexagon, index + 1);
    else
      leafs++;
  }
}

Entry *makehexagon(unsigned long n, long deviation) {
  unsigned long i, j;
  unsigned long num_rows = 2 * n - 1;
  unsigned long num_values = 3 * n * n - 3 * n + 1;

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
    if (i + 1 > n)
      start = i + 1 - n;
    if (i + 1 < n)
      end = n + i;
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
  unsigned long side_length;
  long deviation;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <side_length> <deviation> <value> ... <value>\n",
            argv[0]);
    exit(1);
  }
  side_length = strtoul(argv[1], NULL, 10);
  if (side_length < 1) {
    fprintf(stderr, "order must be >=1\n");
    exit(1);
  }
  deviation = strtol(argv[2], NULL, 10);
  Entry *hexagon = makehexagon(side_length, deviation);
  for (i = 3; i < argc; i++) {
    while (hexagon[j].id < 0)
      j++;
    hexagon[j].lower_bound = hexagon[j].upper_bound = strtol(argv[i], NULL, 10);
    j++;
  }
  labeling(side_length, deviation, hexagon, 0);
  printf("%lu solution(s), %lu leafs visited\n", solutions, leafs);
  //(void)solve(n, d, vs);
  // printhexagon(n, vs);
  return 0;
}
