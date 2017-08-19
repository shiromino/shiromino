# Configurables
BIN_PATH = bin
BIN_NAME = game

CC ?= gcc

SRC_PATH = src
OBJ_PATH = obj

# Do it Do it

INCLUDES = 
COMPILER_FLAGS = -Wall -g
LINK_FLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lm

CFLAGS = $(COMPILER_FLAGS)
ifeq ($(OS), Windows_NT)
	CFLAGS += -lmingw32 -lSDL2main
endif
CFLAGS += $(LINK_FLAGS)

SRC = $(wildcard $(SRC_PATH)/*.c)
# SRC = $(shell find $(SRC_PATH) -name '*.c' -printf '%T@\t%p\n' | sort -k 1nr | cut -f2-)

OBJ = $(SRC:$(SRC_PATH)/%.c=$(OBJ_PATH)/%.o)
DEP = $(OBJ:.o=.d)

.PHONY: all
all: dirs game
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

game : $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(BIN_PATH)/$(BIN_NAME)

-include $(DEPS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@echo "Compiling: $< -> $@"
	$(CC) $(CFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

.PHONY: dirs
dirs:
	@mkdir -p $(dir $(OBJ))
	@mkdir -p $(BIN_PATH)

.PHONY: clean
clean:
	@$(RM) -r $(OBJ_PATH)
	@$(RM) -f $(BIN_PATH)/$(BIN_NAME)
