build:
	clang -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -Wall -g -std=c99 ./src/*.c -o renderer -lSDL2

run:
	./renderer

clean:
	rm ./renderer
