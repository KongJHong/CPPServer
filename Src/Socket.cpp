#include "Socket.h"
#include "Utils.h"
/**************************************************************/
//                                                            //
//                    ServerSocket初始化                       //
//                                                            //
//                                                            //
/**************************************************************/

ServerSocket::ServerSocket(int port,const char *ip):
                    m_port(port),m_ip(ip)
{
    memset(&m_addr,'\0',sizeof(sockaddr_in));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);

    if(ip != nullptr)
        m_addr.sin_addr.s_addr = ::inet_addr(ip);
    else
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    m_listenfd = socket(PF_INET,SOCK_STREAM,0);
    if(m_listenfd == -1){
        std::cout<<"create socket error in file <"<<__FILE__<<"> "<<"at "<<__LINE__<<std::endl;
        exit(0);
    }

    setReusePort(m_listenfd);
    setnonblocking(m_listenfd); //防止accept阻塞
}

bool ServerSocket::bind()
{
    int ret = ::bind(m_listenfd,(struct sockaddr*)&m_addr,sizeof(m_addr));
    if(ret == -1){
        std::cout<<"bind  error in file <"<<__FILE__<<"> "<<"at "<<__LINE__<<std::endl;
        return false;
    }
    return true;
}

bool ServerSocket::listen()
{
    int ret = ::listen(m_listenfd,1024);
    if(ret == -1){
        if(ret == -1){
        std::cout<<"listen error in file <"<<__FILE__<<"> "<<"at "<<__LINE__<<std::endl;
        return false;
        }
    }
    return true;
}

int  ServerSocket::accept(ClientSocket& clientSocket) const
{
    //WARNING: 逻辑由epoll_wait完成
    int clntfd =::accept(m_listenfd,NULL,NULL);
    
    if(clntfd < 0)
    {
        if((errno == EWOULDBLOCK) || (errno == EAGAIN))
            return clntfd;
        std::cout << "accept error in file <" << __FILE__ << "> "<< "at " << __LINE__ << std::endl;
        std::cout << "clientfd:" << clntfd << std::endl;
    }
    
    std::cout << "accept a client： " << clntfd << std::endl;
    clientSocket.m_clntfd = clntfd;
    return clntfd;
}

void ServerSocket::close()
{
    if(m_listenfd >= 0){
        ::close(m_listenfd);
        std::cout << "定时器超时关闭, 文件描述符:" << m_listenfd << std::endl;
        m_listenfd = -1;
    }
}

ServerSocket::~ServerSocket(){
    close();
}


/**************************************************************/
//                                                            //
//                    ClientSocket                            //
//                                                            //
//                                                            //
/**************************************************************/

void ClientSocket::close(){
    if(m_clntfd >= 0){
        std::cout << "文件描述符关闭: " << m_clntfd <<std::endl;
        ::close(m_clntfd);
        m_clntfd = -1;
    }
}

ClientSocket::~ClientSocket(){
    close();
}

