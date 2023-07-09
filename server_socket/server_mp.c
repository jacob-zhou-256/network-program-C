#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char* argv[]){
    if(argc != 2){
        exit(0);
    }
    // 1.创建服务端监听的socket(文件描述符)
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

    // 4.接收
    while(1){
        struct sockaddr_in clientaddr;
        socklen_t len = sizeof(clientaddr);
        int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
        if(clientfd == -1){
            perror("accept");
            exit(0);
        }

        if(fork() > 0){ // 父进程返回到循环顶部: 父进程关闭clientfd, 子进程关闭listenfd
            close(clientfd); // 有一个疑问, 如果添加了这一行代码: 当有多个客户端发送数据时, 每次创建的文件描述符clientfd都是相同的, 这个是为什么了?
            continue;
        }

        close(listenfd);

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
                printf("ret=%d.\n", ret);
                break;
            }else{
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "eventfd=%d,size=%d", clientfd, len);
                write(clientfd, buffer, sizeof(buffer));
                printf("回复: ok.\n");
            }
        }
        printf("客户端socket=%d已断开.\n", clientfd);
        exit(0);    // 子进程通信完成后退出
    }
    return 0;
}