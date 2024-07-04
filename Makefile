# Makefile for compiling main.c with smolar.c and smolar.h
CC = gcc
CFLAGS = -Wall -O2  # Compiler flags, added -O2 for optimization
LDFLAGS = -lm       # Linker flags, added -lm for math library
TARGET = main       # Output executable name

# List of source files
SRCS = main.c smolar.c

# List of object files
OBJS = $(SRCS:.c=.o)

# Target to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Dependencies for each object file
main.o: main.c smolar.h
	$(CC) $(CFLAGS) -c main.c

smolar.o: smolar.c smolar.h
	$(CC) $(CFLAGS) -c smolar.c

# Clean target to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony target to prevent conflicts with files of the same name
.PHONY: clean