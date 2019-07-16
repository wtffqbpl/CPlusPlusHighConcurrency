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
FILELIST   := $(wildcard $(SRC_PATH)/*.cc)
NOTDIRLIST := $(notdir $(FILELIST))
OBJLIST    := $(patsubst %.cc, $(BUILD_PATH)/%.o, $(NOTDIRLIST))
TARGET     := $(addprefix $(BUILD_PATH)/, promiseDemo)

all: $(TARGET)

$(TARGET): $(OBJLIST)
# $(TARGET):
	@echo $(FILELIST)
	@echo $(NOTDIRLIST)
	@echo $(OBJLIST)
	$(CC) -o $@ $^


$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cc
	@if test ! -d "$(BUILD_PATH)"; then \
	    mkdir -p $(BUILD_PATH); \
	fi
	$(CC) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf build

