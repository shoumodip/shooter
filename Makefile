shooter: src/main.c src/game.h src/game.c
	clang `pkg-config --cflags raylib` -o shooter src/main.c src/game.c `pkg-config --libs raylib` -lm
