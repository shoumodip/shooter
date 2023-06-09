.PHONY: all
all: shooter assets/shooter.wasm

shooter: src/main.c src/game.h src/game.c
	clang `pkg-config --cflags raylib` -o shooter src/main.c src/game.c `pkg-config --libs raylib` -lm

assets/shooter.wasm: src/game.h src/game.c
	clang -nostdlib --target=wasm32 -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined -o assets/shooter.wasm src/game.c
