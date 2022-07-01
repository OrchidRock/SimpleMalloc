.PHONY: clean


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

CC = gcc
DEBUG  = -DDEBUG=7
CFALGS = -Wall $(DEBUG)


TARGET = SimpleMallocTest


$(TARGET): $(OBJECTS)
	$(CC) $(CFALGS) -o $@ $^

%.o:%.c
	$(CC) $(CFALGS) -o $@ -c $<

clean:
	-rm -rf $(OBJECTS) $(TARGET)
