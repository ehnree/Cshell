CC=gcc
CFLAGS=-I -Wall -std=c99

#DEPS = hellofunc.h

# %.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

# make: main.o
# 	$(CC) -o shell main.o -I.
make:
	$(CC) main.c -o shell $(CFLAGS)

clean:
	rm -f *.o *~ core* shell
