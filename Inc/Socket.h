/**
 * Socket类: 封装监听socket和客户端socket
 * 一个用于服务器监听Socket
 * 一个用于保存客户端信息的Socket
 */

#include "CommonHead.h"

#pragma once
class ClientSocket;

class ServerSocket
{
public:

    ServerSocket(int port = DEFAULT_PORT,const char *ip = nullptr);

    ~ServerSocket();

    bool bind();    //封装server::bind

    bool listen();  //封装server::listen

    void close();

    int  accept(ClientSocket&) const; //封装server::accept

public:

    sockaddr_in m_addr; //socket信息

    int m_listenfd;     //监听socket

    int m_epollfd;      //epoll句柄

    int m_port;         //监听端口

    const char *m_ip;    //IP地址

};

class ClientSocket
{
public:

    ClientSocket():m_clntfd(-1){}

    void close();

    ~ClientSocket();

public:

    socklen_t m_clnt_sz;//客户端socket信息长度

    sockaddr_in m_addr; //客户端socket信息

    int m_clntfd;       //客户端句柄,默认为-1
};