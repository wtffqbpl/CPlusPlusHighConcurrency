###########################################################
#
#    MAKEFILE
#
###########################################################

# Compiler info.
CC = g++
CPPFLAGS = -Wall -std=c++11 -ggdb -c

# Project path info.
SRC_PATH   := src
INC_PATH   := inc
BUILD_PATH := build
BIN_PATH   := bin
FILELIST   := $(wildcard $(SRC_PATH)/*.cc)
NOTDIRLIST := $(notdir $(FILELIST))
OBJLIST    := $(patsubst %.cc, $(BUILD_PATH)/%.o, $(NOTDIRLIST))
TARGET     := $(addprefix $(BIN_PATH)/, packageDemo)

all: preBuild $(TARGET)

.PHONY: preBuild
preBuild:
	@if test ! -d "$(BUILD_PATH)"; then mkdir -p $(BUILD_PATH); fi
	@if test ! -d "$(BIN_PATH)"; then mkdir -p $(BIN_PATH); fi

$(TARGET): $(OBJLIST)
# $(TARGET):
	$(CC) -o $@ $^


$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cc
	$(CC) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	-rm -rf $(BUILD_PATH)
	-rm -rf $(BIN_PATH)

