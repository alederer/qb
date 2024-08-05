#!/usr/bin/env bash
clang++ Board.cpp Card.cpp CardDb.cpp ResolveQueue.cpp AI.cpp MCTS.cpp Serialize.cpp main.cpp \
	-std=c++14 -g -O0 \
	-Wno-writable-strings \
	-DQB_DEBUG -DQB_VERBOSE \
	-o ../build/QB.out
