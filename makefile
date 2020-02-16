all:
	gcc -o main main.c attacks.c definitions.c movegen.c position.c lookup.c util.c perft.c uci.c -O3
debug:
	gcc -o main main.c attacks.c definitions.c movegen.c position.c lookup.c util.c perft.c uci.c -O3
scratch:
	gcc -o scratch scratch.c attacks.c definitions.c movegen.c position.c lookup.c util.c perft.c uci.c -O3
clean:
	rm main debug