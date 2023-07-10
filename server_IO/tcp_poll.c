#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAXNFDS 1024

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("usage: ./tcp_poll port\n");
        exit(0);
    }

    // 1.创建服务端连接的文件描述符
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket");
        exit(0);
    }

    // 2.绑定
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        perror("bind");
        exit(0);
    }

    // 3.监听
    if(listen(listenfd, 128) == -1){
        perror("listen");
        exit(0);
    }else{
        printf("listenfd=%d.\n", listenfd);
    }

    // fds存放需要监视的socket
    struct pollfd fds[MAXNFDS];
    int maxfd;

    // 4.初始化
    for( int i=0; i<MAXNFDS; i++ ){
        fds[i].fd = -1;
    }
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;
    maxfd = 0;

    while(1){
        int ret = poll(fds, maxfd+1, -1);
        if(ret == -1){
            perror("poll");
            exit(0);
        }else if(ret == 0){     // 超时
            printf("poll() timeout.\n");
            continue;
        }
        if(fds[0].revents&POLLIN){
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);
            int clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
            if(clientfd < 0){
                perror("accept");
                exit(0);
            }
            printf("socket=%d connected ok.\n", clientfd);
            int i;
            for( int i=0; i<MAXNFDS; i++ ){
                if(fds[i].fd == -1){
                    fds[i].fd = clientfd;
                    fds[i].events = POLLIN;
                    break;
                }
            }
            maxfd = maxfd >= clientfd ? maxfd : clientfd;
        }
        int i;
        for( i=1; i<=maxfd; i++ ){
            if(fds[i].revents&POLLIN){
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                int len = read(fds[i].fd, buffer, sizeof(buffer));
                printf("接收: %s\n", buffer);
                if(len == -1){
                    perror("read");
                    exit(0);
                }else if(len == 0){
                    printf("client%d closed.\n", fds[i].fd);
                    fds[i].fd = -1;
                    close(fds[i].fd);
                }else{
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, "eventfd=%d, size=%d", fds[i].fd, len);
                    write(fds[i].fd, buffer, sizeof(buffer));
                    printf("%s: ok.\n\n", buffer);
                }
            }
        }
    }
    close(fds[0].fd);
    return 0;
}