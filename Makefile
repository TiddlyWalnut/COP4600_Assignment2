# Makefile for input and output modules

# Compiler flags
CC = gcc
CFLAGS = -Wall -Werror

# Object files
INPUT_OBJ = input.o
OUTPUT_OBJ = output.o

# Target programs
INPUT_PROG = input
OUTPUT_PROG = output

# Default target
all: $(INPUT_PROG) $(OUTPUT_PROG)

# Input module target
$(INPUT_PROG): $(INPUT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Output module target
$(OUTPUT_PROG): $(OUTPUT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Object file dependencies
input.o: input.c
	$(CC) $(CFLAGS) -c $<

output.o: output.c
	$(CC) $(CFLAGS) -c $<

# Clean target
clean:
	rm -f $(INPUT_OBJ) $(OUTPUT_OBJ) $(INPUT_PROG) $(OUTPUT_PROG)
