/**
 * 关键类：用于封装服务器核心类
 * 一个状态机
 * Socket类创建--->线程池创建--->Epoll创建
 * 请求逻辑
 */


#pragma once
#include <memory>



#include "Socket.h"
#include "HttpData.h"
#define BUFFER_SIZE 2048

/* 关键类：用于封装服务器核心类 */
class HttpServer{
public:
    //访问状态机
    enum FileState{ 
        FILE_OK = 0,
        FILE_NOT_FOUND,
        FILE_FORBIDDEN 
    };
    
    using shared_httpData = std::shared_ptr<HttpData>;

public:
    /* server socket伴随HttpServer创建而初始化 */
    explicit HttpServer(int port = DEFAULT_PORT,const char *ip = nullptr):
                servSocket(port,ip)
    {
        assert(servSocket.bind());
        assert(servSocket.listen()); 
    }

    /* Http服务类主逻辑 */
    void run(int thread_num,int max_request = 10000);

    /* 请求处理主力函数，主要作为回调函数 */
    void do_request(std::shared_ptr<void> arg);

private:

    /**
     * 处理请求头部 
     * */
    void header(shared_httpData);

    /**
     * CGI逻辑：文件返回 
     * 参数1是客户端信息
     * 参数2是字段名
     * 任务逻辑：getMime->static_file->send
     * */
    FileState static_file(shared_httpData,const char *);

    /**
     * 请求文件返回 
     * 任务逻辑：getMime->static_file->send
     * */
    void send(shared_httpData,FileState);

    /**
     * 获取Mime 清理?后的参数和文件后缀，然后设置path到response 
     * 任务逻辑：getMime->static_file->send
     * */ 
    void getMime(shared_httpData);

    /* 内部实现初始化listenfd */
    ServerSocket servSocket;
};