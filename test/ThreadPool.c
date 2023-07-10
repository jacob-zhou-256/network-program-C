#include <stdio.h>
#include <stdlib.h>
#include "ThreadPool.h"

struct ThreadPool* createThreadPool(int minNum, int maxNum, int queueSize){
    struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));

    pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t));
    
    pool->minNum = minNum;
    pool->maxNum = maxNum;
    pool->busyNum = 0;
    pool->liveNum = minNum;
    pool->exitNum = 0;

    pthread_mutex_init(&pool->mutexPool, NULL);
    pthread_mutex_init(&pool->mutexBusy, NULL);
    pthread_cond_init(&pool->notFull, NULL);
    pthread_cond_init(&pool->notEmpty, NULL);

    // 任务队列
    pool->taskQ = (struct Task*)malloc(sizeof(struct Task));
    pool->queueCapacity = queueSize;
    pool->queueSize = 0;
    pool->queueFront = 0;
    pool->queueRear = 0;

    pool->shutdown = 0;

    // 创建线程
    pthread_create(&pool->managerID, NULL, manager, pool);
    for( int i=0; i<minNum; i++ ){
        pthread_create(&pool->threadIDs[i], NULL, worker, pool);
    }

    return pool;
}

