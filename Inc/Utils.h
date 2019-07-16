#pragma once
#include <pthread.h>
#include <string>
/* 设置fd非阻塞 */
int setnonblocking(int fd);

/* 检查文件路径是否合法 */
bool check_base_path(char *basePath);

/* 设置 重用本地地址 */
bool setReusePort(int fd);

/* 删除字符串左空格 */
std::string &ltrim(std::string &);

/* 删除字符串右空格 */
std::string &rtrim(std::string &);

/* 删除字符串左右空格 */
std::string &trim(std::string &);

/* 互斥锁 */
class MutexLock{
public:
    /* 禁止拷贝构造 */
    MutexLock(const MutexLock&) = delete;
    MutexLock & operator=(const MutexLock&) = delete;


    MutexLock(){
       // std::cout<<"lock initialize"<<std::endl;
        if(pthread_mutex_init(&m_mutex,NULL) != 0){
            throw std::exception();
        }
    }

    ~MutexLock(){
      //  std::cout<<"lock destruction"<<std::endl;
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock(){
     //   std::cout<<"lock down"<<std::endl;
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock(){
     //   std::cout<<"lock off"<<std::endl;
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

private:
    pthread_mutex_t m_mutex;
};


/* 条件变量，线程间同步 */
class Condition
{
public:
    /* 禁止拷贝构造 */
    Condition(const Condition&) = delete;
    Condition& operator=(const Condition&) = delete;


    Condition()
    {
        if(pthread_mutex_init(&m_mutex,NULL) != 0)
            throw std::exception();
    
        if(pthread_cond_init(&m_cond,NULL) != 0)
        {
            //构造函数一旦出现了问题，就应该立即释放已经分配了的资源
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }

    /* 销毁条件变量 */
    ~Condition()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }

    /* 等待条件变量 */
    bool wait()
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond,&m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    /* 唤醒等待条件变量的线程 */
    bool notify()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

    /* 广播唤醒 */
    bool notifyAll()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
    

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};