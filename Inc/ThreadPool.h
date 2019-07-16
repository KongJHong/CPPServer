/**
 * 线程池类：
 * 负责线程池的管理，传入的消息是该消息的回调函数，方便线程池管理
 * 线程池同时也管理线程
 * 
 */
#pragma once
#include <memory>
#include <list>
#include <functional>
#include <exception>

#include "Utils.h"


//线程实际执行的方法类，是HttpServer::do_request
struct ThreadTask{
    //回调函数，实际传的是HttpServer::do_request
    std::function<void(std::shared_ptr<void>)> process; 
    //实际应该是HttpData对象
    std::shared_ptr<void> arg;                  
};


class ThreadPool{
public:
    
    ThreadPool(int thread_number,int max_requests);

    ~ThreadPool();

    //请求插入线程池队列，回调参数，回调方法
    bool append(std::shared_ptr<void> arg,std::function<void(std::shared_ptr<void>)> fun);

private:

    static void* worker(void *args);

    void run();

private:
    //线程同步互斥
    MutexLock m_mutex;
    Condition m_cond;

    /* 线程池中线程数 */
    int m_thread_number;

    /* 请求队列中允许的最大请求数 */
    int m_max_requests;

    /* 线程池数组,用于索引 */
    pthread_t *m_threads;

    /* 请求队列 */
    std::list<ThreadTask> m_request_queue;

    /* 是否结束线程 */
    bool m_shutdown;
};
