CC      = clang
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11 -g
CFLAGS += $(shell pkg-config --cflags sdl2)
LDFLAGS  = $(shell pkg-config --libs sdl2)

SRCS   = $(shell find . -name "*.c" -not -path "./.git/*" -not -path "./tests/*")
OBJS   = $(SRCS:.c=.o)
TARGET = civ

TEST_SRCS = $(shell find tests -name "*.c" 2>/dev/null)
TEST_BIN  = tests/run_tests

.PHONY: all cli sdl req test clean reset

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

cli: $(TARGET)
	./$(TARGET) --mode cli

sdl: $(TARGET)
	./$(TARGET) --mode sdl

req:
	sudo apt-get install -y libsdl2-dev

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS) $(filter-out ./main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	find . -name "*.o" -delete
	rm -f $(TARGET) $(TEST_BIN)

reset: clean
	rm -f saves/*.sav scores.txt
