CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = civ
SRCS    = main.c game.c gen.c units.c cities.c barbarians.c tech.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
