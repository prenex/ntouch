all:
	gcc -std=c89 -pedantic ntouch.c -O2 -o ntouch.out
debug:
	gcc -std=c89 -pedantic ntouch.c -g -o ntouch.out
install:
	cp -pr ntouch.out /usr/local/bin/ntouch
