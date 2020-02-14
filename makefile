all:
	gcc -o main main.c attacks.c definitions.c movegen.c position.c lookup.c util.c -O3
debug:
	gcc -o debug main.c attacks.c definitions.c movegen.c position.c lookup.c util.c -g
clean:
	rm main