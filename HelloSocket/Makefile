###########################################################
#
#    MAKEFILE
#
###########################################################

# Compiler info.
CC = g++
CPPFLAGS = -Wall -std=c++11 -ggdb

# Project path info.
SRC_PATH = ./src
INC_PATH = ./inc
BUILD_PATH = ./build

all: Thread


Thread: Thread.o
	$(CC) -o $(BUILD_PATH)/$@ $(BUILD_PATH)/$^

Thread.o: $(SRC_PATH)/Thread.cc
	$(CC) $(CPPFLAGS) -I$(INC_PATH) -o $(BUILD_PATH)/$@ -c $^
