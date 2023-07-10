#pragma once
#include <pthread.h>

// 任务结构体
struct Task{
    void (*function)(void* arg);
    void* arg;
};

// 线程池结构体
struct ThreadPool{
    struct Task* taskQ;
    int queueCapacity;
    int queueSize;
    int queueFront;
    int queueRear;

    pthread_t managerID;
    pthread_t* threadIDs;
    int minNum;     // 最小线程数量
    int maxNum;     // 最大线程数量
    int busyNum;    // 忙的线程数量
    int liveNum;    // 存活的线程数量
    int exitNum;    // 要销毁的线程数量

    pthread_mutex_t mutexPool;  // 锁整个线程池
    pthread_mutex_t mutexBusy;  // 锁busyNum变量
    pthread_cond_t notFull;     // 任务队列是不是满了
    pthread_cond_t notEmpty;    // 任务队列是不是空了

    int shutdown;   // 是否要销毁线程池, 销毁为1, 不销毁为0
};

struct ThreadPool* createThreadPool(int minNum, int maxNum, int queueSize);

void* manager(void* arg);

void* worker(void* arg);