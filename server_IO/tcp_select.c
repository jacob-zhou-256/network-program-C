#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main(int argc, char* argv[]){
    if(argc != 2){
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
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("bind");
        exit(-1);
    }

    // 3.监听
    // 3.监听
    if(listen(listenfd, 128) == -1){
        perror("listen");
        exit(-1);
    }else{
        printf("listenfd=%d.\n", listenfd);
    }

    fd_set fdset;  // 创建存放多个文件描述符的数组
    FD_ZERO(&fdset);    // 将文件描述符fd的标志为置0
    FD_SET(listenfd, &fdset);   // 将监听的文件描述符listenfd设置为1
    int maxfd = listenfd;   // 获取数组里的最大的文件描述符的数值
    while(1){
        fd_set tmp = fdset;    // 用tmp进行内核态的操作
        int ret = select(maxfd+1, &tmp, NULL, NULL, NULL);
        if(ret == -1){
            perror("select");
            break;
        }
        
        // 文件描述符的缓冲区的数据发生了变化
        for( int sockfd=0; sockfd<=maxfd; sockfd++ ){
            if(FD_ISSET(sockfd, &tmp) <= 0){
                continue;
            }
            if(sockfd == listenfd){   // 有新连接
                struct sockaddr_in clientaddr;
                socklen_t socklen = sizeof(clientaddr);
                int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &socklen);           
                FD_SET(clientfd, &fdset);   // 把新的客户端socket加入集合
                maxfd = maxfd>clientfd?maxfd:clientfd;  // 更新数组里的最大的文件描述符的数值
            }else{                   // 客户端有数据发送过来或客户端的socket连接被断开
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                int len = read(sockfd, buffer, sizeof(buffer));   // 读取客户端发送过来的数据
                printf("接收: %s\n", buffer);
                if(len == -1){
                    perror("read");
                    exit(-1);
                }
                if(len == 0){
                    printf("client closed.\n");
                    FD_CLR(sockfd, &fdset);
                    if(sockfd == maxfd){
                        for( int j=0; j<=maxfd; j++){
                            if(FD_ISSET(j, &fdset)){
                                maxfd = j;
                                break;
                            }
                        }
                    }
                    close(sockfd);
                }else{
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, "eventfd=%d,size=%d", sockfd, len);
                    write(sockfd, buffer, sizeof(buffer));  // 发送数据到客户端
                    printf("%s: ok.\n\n", buffer);
                }
            }
        }
    }
    close(listenfd);
    return 0;
}