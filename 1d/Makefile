# You can compile with either gcc or g++
CC = g++
# CC = gcc
CFLAGS = -I. -Wall -lm -ldl -no-pie
# disable the -DNDEBUG flag for the printing the freelist
OPTFLAG = -O2


test1: test1.cc
	$(CC) -o test1 thread.o test1.cc libinterrupt.a $(CFLAGS)

clean:
	rm -f deli
