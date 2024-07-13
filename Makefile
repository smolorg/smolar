# Makefile for compiling smolar.c into a shared library and main.c as an executable

CC = gcc
CFLAGS = -Wall -O2 -g -fPIC # Added -fPIC for position-independent code
LDFLAGS = -lm -Wl,-rpath,. # Added -Wl,-rpath,. to set runtime library path

# Targets
LIBRARY = libsmolar.so     # Shared library name
EXECUTABLE = main          # Executable name

# Source files
LIB_SRC = smolar.c
MAIN_SRC = main.c

# Object files
LIB_OBJ = $(LIB_SRC:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)

# Default target
all: $(LIBRARY) $(EXECUTABLE)

# Target to build the shared library
$(LIBRARY): $(LIB_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

# Target to build the executable
$(EXECUTABLE): $(MAIN_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) -L. -lsmolar $(LDFLAGS)

# Compile smolar.c into an object file
$(LIB_OBJ): $(LIB_SRC) smolar.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main.c into an object file
$(MAIN_OBJ): $(MAIN_SRC) smolar.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove object files, library, and executable
clean:
	rm -f $(LIB_OBJ) $(MAIN_OBJ) $(LIBRARY) $(EXECUTABLE)

# Phony target to prevent conflicts with files of the same name
.PHONY: all clean