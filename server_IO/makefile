all: tcp_select client

tcp_select: tcp_select.c
	gcc -o tcp_select tcp_select.c

client: client.c
	gcc -o client client.c

clean:
	rm -rf tcp_select client