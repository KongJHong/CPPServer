#include "ThreadPool.h"
#include <iostream>
#include <sys/prctl.h>
ThreadPool::ThreadPool(int thread_number,int max_requests):
                m_thread_number(thread_number),m_max_requests(max_requests),
                m_shutdown(false),m_threads(nullptr)
{
    if((thread_number <= 0) || (max_requests <= 0))
        throw std::exception();

    m_threads = new pthread_t[m_thread_number];

    if(!m_threads)
        throw std::exception();

    /* 创建thread_number个线程,并将它们都设置为脱离线程 */
    for(int i = 0;i < thread_number;i++)
    {
        printf("create the %dth thread\n",i+1);
        if(pthread_create(m_threads+i,NULL,worker,this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

ThreadPool::~ThreadPool()
{
    delete[] m_threads;
    m_shutdown = true;
}


bool ThreadPool::append(std::shared_ptr<void> arg,std::function<void(std::shared_ptr<void>)> fun)
{
    if(m_shutdown){
        std::cout<<"ThreadPool has shutdown"<<std::endl;
        return false;
    }

    ThreadTask threadTask;
    threadTask.arg = arg;
    threadTask.process = fun;

    m_mutex.lock();
    if(m_request_queue.size() > m_max_requests)
    {
        m_mutex.unlock();
        return false;
    }

    m_request_queue.push_back(threadTask);

    m_cond.notify();
    m_mutex.unlock();
    

    return true;
}



void* ThreadPool::worker(void *args)
{
    ThreadPool *pool = static_cast<ThreadPool*>(args);
    if(pool == nullptr)
        return nullptr;

    //给线程命名
    prctl(PR_SET_NAME,"EventLoopThread");
    
    pool->run();
    return pool;
}

void ThreadPool::run()
{
    //WARNING:线程运行函数
    while(!m_shutdown)
    {
        m_cond.wait();
        m_mutex.lock();
        if(m_request_queue.empty())
        {
            m_mutex.unlock();
            continue;
        }
        ThreadTask requestTask = m_request_queue.front();
        m_request_queue.pop_front();
        m_mutex.unlock();

        requestTask.process(requestTask.arg);
    }
}