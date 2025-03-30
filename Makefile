CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = example
SOURCES = example.c simple-lexer.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
