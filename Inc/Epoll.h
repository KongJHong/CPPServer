/**
 * Epoll类的封装:
 * 因为Epoll内核句柄只有一个，所以这是一个工具类，单例或者静态，这里选择静态
 */

#pragma once
#include <memory>
#include <vector>
#include <unordered_map>


#include "Socket.h"
#include "HttpData.h"
#include "Utils.h"
class Epoll {
public:
    /* epoll 初始化--返回epollfd句柄 */
    static int init(int max_event); 

    /* 添加epoll监控句柄，利用HttpData绑定 */
    static bool addfd(int epoll_fd,int fd,__uint32_t events,std::shared_ptr<HttpData>);

    /* 修改epoll监控句柄的事件 */
    static bool modfd(int epoll_fd,int fd,__uint32_t events,std::shared_ptr<HttpData>);

    /* 删除epoll监控句柄的事件 */
    static bool delfd(int epoll_fd,int fd,__uint32_t events);

    /**
     * 代替Epoll_wait及其下工作
     * EPOLLIN时，返回HTTP待转发消息向量（主线程转发给子线程） 
     * 有connect信号内部解决掉
     * */
    static std::vector<std::shared_ptr<HttpData>>
        poll(const ServerSocket &serverSocket,int max_event,int timeout);

    /* 链接处理函数 */
    static void handleConnection(const ServerSocket &serverSocket);



public:
    /** 
     * key-value  
     * fd-HttpData: 文件句柄-HTTPDATA的映射，用来保存客户端连接
     * 同时方便查找
    **/
    static std::unordered_map<int,std::shared_ptr<HttpData>> httpDataMap;

    /* 最大处理事件 */
    static const int MAX_EVENTS;

    /* 事件保存数组 */
    static epoll_event *events;

    /* 默认事件 EPOLLIN EPOLLET EPOLLONESHOT */
    const static __uint32_t DEFAULT_EVENTS;

    /* 定时器容器 */
    static TimerManager m_timerManager;

};