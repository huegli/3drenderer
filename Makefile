build:
	clang -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -Wall -std=c99 ./src/main.c -o renderer -lSDL2

run:
	./renderer

clean:
	rm ./renderer
