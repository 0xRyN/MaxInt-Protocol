all:
	gcc -Wall -pthread -g -o server server.c
	gcc -Wall -pthread -g -o client1 client1.c
	gcc -Wall -pthread -g -o client2 client2.c

clean: 
	rm -f server
	rm -f client1
	rm -f client2