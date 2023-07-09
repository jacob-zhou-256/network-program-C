#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char* argv[]){
    if(argc != 2){
        exit(0);
    }
    // 1.创建服务端用于监听的socket(文件描述符)
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket");
        exit(0);
    }

    // 2.绑定
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        perror("bind");
        exit(0);
    }

    // 3.设置监听
    if(listen(listenfd, 128) == -1){
        perror("listen");
        exit(0);
    }else{
        printf("listenfd=%d.\n", listenfd);
    }

    // 4.阻塞等待并接受客户端连接
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
    if(clientfd == -1){
        perror("accept");
        exit(0);
    }else{
        close(listenfd);
        char ip[24] = {0};
        printf("客户端的IP地址: %s, 端口: %d\n",
            inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, ip, sizeof(ip)),
            ntohs(clientaddr.sin_port));
    }
    
    // 5.和客户端通信
    while(1){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int ret = read(clientfd, buffer, sizeof(buffer));
        int len = sizeof(buffer);
        printf("接收: %s\n", buffer);
        if(ret < 0){
            perror("read");
            exit(0);
        }else if(ret == 0){
            printf("客户端断开了...\n");
            break;
        }else{
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "eventfd=%d,size=%d", clientfd, len);
            write(clientfd, buffer, sizeof(buffer));
            printf("回复: ok.\n");
        }
    }
    close(clientfd);
    return 0;
}