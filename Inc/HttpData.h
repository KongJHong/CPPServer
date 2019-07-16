#pragma once

#include <memory>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Timer.h"
#include "Socket.h"

//warning enable_shared_from_this可以不加
/**
 * 对每次数据传入的封装
 * 包括：
 * 客户端链接信息，请求信息，返回信息，所属epoll容器,定时器保存长连接
 */

class TimerNode;

class HttpData:public std::enable_shared_from_this<HttpData>{
public:
    HttpData():epoll_fd(-1){}

public:

    /* 该数据的请求 */
    std::shared_ptr<HttpRequest> m_request;

    /* 该数据的响应 */
    std::shared_ptr<HttpResponse> m_response;

    /* 该数据所属的客户端数据 */
    std::shared_ptr<ClientSocket> m_clntSocket;

    /* 该数据属于的epollfd */
    int epoll_fd;

    /* 关闭定时器 */
    void closeTimer();

    /* 设置定时器 */
    void setTimer(std::shared_ptr<TimerNode> timer);
private:
    std::weak_ptr<TimerNode> weak_timer;
};