CFLAGS = -g -Wall -Wextra -std=gnu11

BIN = ferdig_program

all: $(BIN)

ferdig_program: tests.c stringoperations.c 
	  gcc $(CFLAGS) tests.c stringoperations.c -o ferdig_program

clean: 
	rm -f $(BIN)