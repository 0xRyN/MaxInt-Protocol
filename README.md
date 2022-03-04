# C Max Int Protocol

_An implementation of the max int protocol in C with POSIX threads and mutex_

## How to use

1. Compile the three files **client1.c**, **client2.c** and **server.c** using the `make` command.

2. Run the server. You can add a port as an optional argument. Example : `./server` or `./server 8080` (Default port is 8080)

3. Run either of the clients.

- If you run **client2**, you will recieve a NOP message (No user has sent an INT yet).
- If you run **client1**, the server will receive 5 random integers from 0 to 10000 exclusive. The server will keep the highest score + highest user data.

PS : The IP adresses that the clients connect to are in a **#define** macro. You can change that easily for each client.
