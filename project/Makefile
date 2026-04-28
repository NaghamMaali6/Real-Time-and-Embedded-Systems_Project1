# =================
# Compiler & flags
# =================
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lGL -lGLU -lglut -lm

# ============
# Directories
# ============
SRC_DIR = src
OBJ_DIR = obj
BIN = app

# ======
# Files
# ======
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# ===============
# Default target
# ===============
all: $(BIN)

# ======================
# Link final executable
# ======================
$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

# ==================
# Compile .c -> .o
# ==================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================
# Create obj directory
# =====================
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ====
# Run 
# ====
run: all
	./$(BIN)

# ======
# Clean
# ======
clean:
	rm -rf $(OBJ_DIR) $(BIN)