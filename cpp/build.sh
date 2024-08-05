#!/usr/bin/env bash
source ~/SPACE/emsdk/emsdk_env.sh
~/SPACE/emsdk/upstream/emscripten/tools/webidl_binder QB.idl ../build/glue
em++ Board.cpp Card.cpp CardDb.cpp ResolveQueue.cpp AI.cpp MCTS.cpp Serialize.cpp main.cpp \
	-DWASM -DQB_DEBUG -O3 -s EXPORT_ES6=1 -s ALLOW_MEMORY_GROWTH=1 \
	-s SINGLE_FILE=1 \
	-Wno-writable-strings \
	--post-js ../build/glue.js \
	-o ../build/QB.js 
