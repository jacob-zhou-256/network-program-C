#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAXEVENTS 100

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("usage: ./tcp_epoll port.\n");
        exit(-1);
    }

    // 1.创建服务端监听的文件描述符
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket");
        exit(-1);
    }

    // 2.绑定
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        perror("bind");
        exit(-1);
    }

    // 3.监听
    if(listen(listenfd, 128) == -1){
        perror("listen");
        exit(-1);
    }else{
        printf("listenfd=%d.\n", listenfd);
    }

    // 创建一个描述符
    int epollfd = epoll_create(1);

    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    while(1){
        struct epoll_event events[MAXEVENTS];
        int ret = epoll_wait(epollfd, events, MAXEVENTS, -1);
        if(ret < 0){        // 失败
            perror("epoll_wait");
            exit(-1);
        }else if(ret == 0){ // 超时
            printf("epoll_wait() timeout.\n");
            continue;
        }
        // 否则为, 有事件发生
        for( int i=0; i<ret; i++ ){
            if(events[i].data.fd == listenfd){
                int clientfd = accept(listenfd, NULL, NULL);
                if(clientfd < 0){
                    perror("accept");
                    exit(-1);
                }
                // 将文件描述符设置为非阻塞
                int flag = fcntl(clientfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(clientfd, F_SETFL, flag);

                memset(&ev, 0, sizeof(ev));
                ev.data.fd = clientfd;
                ev.events = EPOLLIN|EPOLLET;    // 通信的文件描述符检测读缓冲区数据的时候设置为边沿模式
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
                printf("socket=%d connect ok.\n", clientfd);
            }else{
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                while(1){
                    int len = read(events[i].data.fd, buffer, sizeof(buffer));
                    if(len < 0){
                        if(errno == EAGAIN){
                            printf("数据读完了...\n");
                            break;
                        }else{
                            perror("read");
                            exit(-1);
                        }
                    }else if(len == 0){
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(events[i].data.fd);
                        printf("client%d closed.\n", events[i].data.fd);
                        break;
                    }else{
                        printf("接收: %s\n", buffer);
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "eventfd=%d,size=%d", events[i].data.fd, len);
                        write(events[i].data.fd, buffer, sizeof(buffer));  // 发送数据到客户端
                        printf("%s: ok.\n\n", buffer);
                    }
                }
            }
        }
    }
    close(epollfd);
    close(listenfd);
    return 0;
}