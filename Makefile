build:
	clang -Wall -std=c99 ./src/main.c -o renderer

run:
	./renderer

clean:
	rm ./renderer
