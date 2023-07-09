all: server server_mp server_mt client

server: server.c
	gcc -o server server.c

server_mp: server_mp.c
	gcc -o server_mp server_mp.c

server_mt: server_mt.c
	gcc -o server_mt server_mt.c -lpthread

client: client.c
	gcc -o client client.c

clean:
	rm -rf server server_mp server_mt client