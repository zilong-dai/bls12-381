CC = g++
CFLAGS = -std=c++17 -w -o3 

# Name of the library
LIB_NAME = bls

# Assuming src is a defined variable or replace with an actual path
LIB_SOURCES = $(wildcard src/*.cpp)

# Object files for the library
LIB_OBJECTS = $(patsubst %.cpp,%.o,$(LIB_SOURCES))

# Header files for the library
INCLUDE_HEADERS = include

# Source files for the test program
TEST_SOURCES = test/unittests.cpp

# Object files for the test program
TEST_OBJECTS = $(patsubst %.cpp,%.o,$(TEST_SOURCES))

all: lib$(LIB_NAME).a unittest

lib$(LIB_NAME).a: $(LIB_OBJECTS)
	ar rcs $@ $^

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INCLUDE_HEADERS)

unittest: $(TEST_OBJECTS) lib$(LIB_NAME).a
	$(CC) $(CFLAGS) $^ -o test/$@ -L. -l$(LIB_NAME) -I$(INCLUDE_HEADERS)

clean:
	rm -f $(LIB_OBJECTS) $(TEST_OBJECTS) $(LIB_NAME).a unittest