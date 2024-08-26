INC = $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs)

build:
	clang $(INC) $(LIBS) -Wall -g -std=c99 ./src/*.c -o renderer

run:
	./renderer

clean:
	rm ./renderer
