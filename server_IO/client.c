#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Using: ./client ip port\n");
        exit(0);
    }

    // 1.创建客户端的socket
    int sockfd;
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(0);
    }

    // 2.连接服务器
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    if((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) == -1){
        perror("connect");
        exit(0);
    }

    // 3.与服务器通信
    for( int i=0; i<5; i++ ){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "这是第%d个超级飞人, 编号为%d.",i+1, i+1);
        
        int ret = write(sockfd, buffer, sizeof(buffer));
        printf("send buffer: %s\n", buffer);
        sleep(1);
        if(ret <= 0){
            perror("write");
            exit(0);
        }else{ 
            memset(buffer, 0, sizeof(buffer));
            ret=read(sockfd, buffer, sizeof(buffer));
            printf("buffer info: %s\n\n", buffer);
        }  
    }

    // 4.关闭socket, 释放资源
    close(sockfd);
    return 0;
}