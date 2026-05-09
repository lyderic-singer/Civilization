CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g -O2
LDFLAGS = -lm

SRC_DIR = src
OBJ_DIR = obj

SRCS = $(SRC_DIR)/game.c \
       $(SRC_DIR)/gen.c  \
       $(SRC_DIR)/units.c \
       $(SRC_DIR)/cities.c \
       $(SRC_DIR)/barbarians.c \
       $(SRC_DIR)/tech.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET = civ

# ── Test sources ──────────────────────────────────────────────
TEST_SRC = tests/test_main.c
TEST_BIN = tests/run_tests

.PHONY: all cli sdl req test clean reset

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

cli: $(TARGET)
	./$(TARGET) --mode cli

sdl: $(TARGET)
	@echo "Mode SDL non implemente (utilisez --mode cli)"
	./$(TARGET) --mode cli

req:
	@echo "Aucune dependance externe requise (C standard + math)."

# ── Tests ─────────────────────────────────────────────────────
test: $(TEST_BIN)
	./$(TEST_BIN)

# Compile all non-main objects for test reuse
TEST_OBJS = $(filter-out $(OBJ_DIR)/game.o, $(OBJS))

$(TEST_BIN): $(TEST_SRC) $(TEST_OBJS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_BIN)

reset: clean
	@echo "Aucun fichier de donnees a supprimer."