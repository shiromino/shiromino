# Configurables
BIN_PATH = bin
BIN_NAME = game

ifdef MINGW
    CXX = i686-w64-mingw32-g++
endif
CXX ?= g++

SRC_PATH = src
OBJ_PATH = obj

# Do it Do it

INCLUDES =

ifdef MINGW
    PKG_CONFIG ?= i686-w64-mingw32-pkg-config
endif
PKG_CONFIG ?= pkg-config

COMPILER_FLAGS = -Wall -Wno-comment -g `${PKG_CONFIG} --cflags sdl2` -DSQLITE_OMIT_LOAD_EXTENSION
LINK_FLAGS = `${PKG_CONFIG} --libs sdl2` -lSDL2_image -lSDL2_mixer -lsqlite3 -lm -lpthread

CFLAGS = $(COMPILER_FLAGS)
ifeq ($(OS), Windows_NT)
	CFLAGS += -lmingw32 -lSDL2main
endif

SRC = $(wildcard $(SRC_PATH)/*.cpp)
# SRC = $(shell find $(SRC_PATH) -name '*.cpp' -printf '%T@\t%p\n' | sort -k 1nr | cut -f2-)

OBJ = $(SRC:$(SRC_PATH)/%.cpp=$(OBJ_PATH)/%.o)
DEP = $(OBJ:.o=.d)

.PHONY: all
all: dirs game
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

game : $(OBJ)
	$(CXX) $(OBJ) $(CFLAGS) $(LINK_FLAGS) -o $(BIN_PATH)/$(BIN_NAME)
ifdef MINGW
	cp /usr/i686-w64-mingw32/bin/*.dll .
endif

-include $(DEPS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	@echo "Compiling: $< -> $@"
	$(CXX) $(CFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

.PHONY: dirs
dirs:
	@mkdir -p $(dir $(OBJ))
	@mkdir -p $(BIN_PATH)

.PHONY: clean
clean:
	@$(RM) -r $(OBJ_PATH)
	@$(RM) -f $(BIN_PATH)/$(BIN_NAME) *.dll
