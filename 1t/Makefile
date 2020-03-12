# You can compile with either gcc or g++
CC = g++ -std=c++98 -Wno-write-strings
# CC = gcc
CFLAGS = -I -WI,--no-as-needed -ldl -lm -no-pie
# disable the -DNDEBUG flag for the printing the freelist
OPTFLAG = -O2

all: test1 test2 test3 test4 test5 test6 test7 test8
test1: test1.cc thread.cc
	$(CC) -o test1 thread.cc test1.cc libinterrupt.a $(CFLAGS)
test2: test2.cc thread.cc
	$(CC) -o test2 thread.cc test2.cc libinterrupt.a $(CFLAGS)
test3: test3.cc thread.cc
	$(CC) -o test3 thread.cc test3.cc libinterrupt.a $(CFLAGS)
test4: test4.cc thread.cc
	$(CC) -o test4 thread.cc test4.cc libinterrupt.a $(CFLAGS)
test5: test5.cc thread.cc
	$(CC) -o test5 thread.cc test5.cc libinterrupt.a $(CFLAGS)
test6: test6.cc thread.cc
	$(CC) -o test6 thread.cc test6.cc libinterrupt.a $(CFLAGS)
test7: test7.cc thread.cc
	$(CC) -o test7 thread.cc test7.cc libinterrupt.a $(CFLAGS)
test8: test8.cc thread.cc
	$(CC) -o test8 thread.cc test8.cc libinterrupt.a $(CFLAGS)

clean:
	rm -f test3
