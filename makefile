CC := cc
objects = main.o

all: main

main: $(objects)

$(objects): %.o: %.c

clean:
	rm -f *.o main