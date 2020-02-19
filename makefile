all:
	gcc -o sasha sasha.c attacks.c definitions.c movegen.c position.c lookup.c util.c uci.c hashtable.c search.c -O3
debug:
	gcc -o debug sasha.c attacks.c definitions.c movegen.c position.c lookup.c util.c uci.c hashtable.c search.c -g
scratch:
	gcc -o scratch scratch.c attacks.c definitions.c movegen.c position.c lookup.c util.c uci.c hashtable.c search.c -O3
clean:
	rm main debug scratch