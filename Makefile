# Makefile for compiling main.c with smolar.c and smolar.h

CC = gcc
CFLAGS = -Wall   # Compiler flags, adjust as needed
TARGET = main       # Output executable name

# List of source files
SRCS = main.c smolar.c

# List of object files
OBJS = $(SRCS:.c=.o)

# Target to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Dependencies for each object file
main.o: main.c smolar.h
	$(CC) $(CFLAGS) -c main.c

smolar.o: smolar.c smolar.h
	$(CC) $(CFLAGS) -c smolar.c

# Clean target to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
