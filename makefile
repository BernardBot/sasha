all:
	gcc -o main main.c attacks.c definitions.c movegen.c position.c lookup.c util.c uci.c hashtable.c -O3
debug:
	gcc -o debug main.c attacks.c definitions.c movegen.c position.c lookup.c util.c uci.c hashtable.c -g
scratch:
	gcc -o scratch scratch.c attacks.c definitions.c movegen.c position.c lookup.c util.c uci.c hashtable.c -O3
clean:
	rm main debug