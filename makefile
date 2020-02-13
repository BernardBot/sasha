all:
	gcc -o main main.c attacks.c definitions.c movegen.c position.c lookup.c util.c -O3
clean:
	rm main