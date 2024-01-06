FILES=HEADER.html Makefile magichex.c reference-output

BUILD := normal

cflags.common := -Wall -Ofast -DNDEBUG -s -finline-functions -funroll-loops
cflags.normal :=
cflags.profile := -fprofile-generate=profdata
cflags.release := -fprofile-use=profdata

ldflags.common := -g
ldflags.normal :=
ldflags.profile := -fprofile-generate=profdata
ldflags.release := -fprofile-use=profdata

CFLAGS := ${cflags.${BUILD}} ${cflags.common}
LDFLAGS := ${ldflags.${BUILD}} ${ldflags.common}

magichex: magichex.o

test: magichex
	./magichex 4 3 14 33 30 34 39 6 24 20

test1: magichex
	./magichex 3 2

test2: magichex
	./magichex 3 0

measure: magichex
	perf stat -e cycles:u -e instructions:u -e branches:u -e branch-misses:u -e L1-dcache-load-misses:u ./magichex 4 3 14 33 30 34 39 6 24 20

speed:
	touch out.txt
	perf stat --repeat 100 -e duration_time ./magichex 4 3 14 33 30 34 39 6 24 20

checkmeasure: magichex
	perf stat -e cycles:u -e instructions:u -e branches:u -e branch-misses:u -e L1-dcache-load-misses:u ./magichex 4 3 14 33 30 34 39 6 24 20 |\
	grep -v solution| \
	awk '/^leafs visited:/ {printf("\0")} /^leafs visited:/,/^$$/ {next} 1'|\
	sort -z|\
	tr '\0' '\n\n' |\
	diff -u reference-output -


dist:
	mkdir effizienz-aufgabe23
	cp -p $(FILES) effizienz-aufgabe23
	tar cfz effizienz-aufgabe23.tar.gz effizienz-aufgabe23

clean:
	rm magichex magichex.o
