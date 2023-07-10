all: tcp_select tcp_poll tcp_epoll tcp_epoll2 client

tcp_select: tcp_select.c
	gcc -o tcp_select tcp_select.c

tcp_poll: tcp_poll.c
	gcc -o tcp_poll tcp_poll.c

tcp_epoll: tcp_epoll.c
	gcc -o tcp_epoll tcp_epoll.c

tcp_epoll2: tcp_epoll2.c
	gcc -o tcp_epoll2 tcp_epoll2.c

client: client.c
	gcc -o client client.c

clean:
	rm -rf tcp_select tcp_poll tcp_epoll tcp_epoll2 client