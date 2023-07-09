#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct SockInfo{
    int fd;
    pthread_t tid;
    struct sockaddr_in clientaddr;
};
struct SockInfo infos[128];

void *task(void *arg);

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

    // 4.数据初始化
    int count = sizeof(infos)/sizeof(struct SockInfo);
    for( int i=0; i<count; i++ ){
        bzero(&infos[i], sizeof(struct SockInfo));
        infos[i].fd = -1;
    }

    // 5.接收
    while(1){
        struct SockInfo* info;
        for( int i=0; i<count; i++ ){
            if(infos[i].fd == -1){
                info = &infos[i];
                break;
            }
            if(i == count-1){
                i--;
                sleep(1);
            }
        }
        socklen_t len = sizeof(struct sockaddr_in);
        int clientfd = accept(listenfd, (struct sockaddr *)&info->clientaddr, &len);
        if(clientfd == -1){
            perror("accept");
            exit(0);
        }
        info->fd = clientfd;
        pthread_create(&info->tid, NULL, task, (void*)info);
    }
    close(listenfd);
    return 0;
}

void *task(void *arg){
    struct SockInfo* info = (struct SockInfo*)arg;
    while(1){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int ret = read(info->fd, buffer, sizeof(buffer));
        int len = sizeof(buffer);
        printf("接收: %s\n", buffer);
        if(ret < 0){
            perror("read");
            return NULL;
        }else if(ret == 0){
            break;
        }else{
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "eventfd=%d,size=%d", info->fd, len);
            write(info->fd, buffer, sizeof(buffer));
            printf("回复: ok.\n");
        }
    }
    close(info->fd);
    printf("客户端socket=%d已断开.\n", info->fd);
    return NULL;
}
