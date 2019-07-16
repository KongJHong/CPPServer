#include "Epoll.h"

/* 静态变量初始化 */
std::unordered_map<int,std::shared_ptr<HttpData>> Epoll::httpDataMap;
const int Epoll::MAX_EVENTS = 10000;
epoll_event* Epoll::events;
const __uint32_t Epoll::DEFAULT_EVENTS = (EPOLLIN | EPOLLET | EPOLLONESHOT);
TimerManager Epoll::m_timerManager;


int Epoll::init(int max_event)
{
    int epollfd = ::epoll_create(max_event);
    if(epollfd == -1){
        std::cout<<"epoll create error"<<std::endl;
        exit(-1);
    }
    events = new epoll_event[max_event];
    return epollfd;
}


bool Epoll::addfd(int epoll_fd,int fd,__uint32_t events,std::shared_ptr<HttpData> HttpData)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    //增加httpDataMap索引
    httpDataMap[fd] = HttpData;
    int ret = ::epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
    if(ret < 0){
        std::cout<<"epoll add error :"<<errno<<std::endl;
        httpDataMap[fd].reset();
        //httpDataMap可以不用删除，重用
        return false;
    }
    return true;
}


bool Epoll::modfd(int epoll_fd,int fd,__uint32_t events,std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    //每次更改的时候都要更新 httpDataMap
    httpDataMap[fd] = httpData;
    int ret = ::epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&event);
    if(ret < 0){
        std::cout<<"epoll mod error :"<<errno<<std::endl;
        httpDataMap[fd].reset();
        //httpDataMap可以不用删除，重用
        return false;
    }
    return true;

}


bool Epoll::delfd(int epoll_fd,int fd,__uint32_t events)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;

    int ret = ::epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&event);
    if(ret < 0){
        std::cout<<"epoll del error :"<<errno<<std::endl;
        return false;
    }

    auto iter = httpDataMap.find(fd);
    if(iter != httpDataMap.end()){
        iter->second.reset();
        httpDataMap.erase(iter);
    }
    return true;
}


void Epoll::handleConnection(const ServerSocket &serverSocket)
{
    std::shared_ptr<ClientSocket> tempClient(new ClientSocket);

    while(serverSocket.accept(*tempClient) > 0){
        //设置成非阻塞
        int ret = setnonblocking(tempClient->m_clntfd);
        
        //保存连接的客户端信息以其相关堆空间的开辟（包括：请求信息，返回新奇，Socket信息，所属epollfd)
        std::shared_ptr<HttpData> sharedHttpData(new HttpData);
        sharedHttpData->m_request = std::shared_ptr<HttpRequest>(new HttpRequest());
        sharedHttpData->m_response = std::shared_ptr<HttpResponse>(new HttpResponse());

        std::shared_ptr<ClientSocket> sharedClientSocket(new ClientSocket());
        sharedClientSocket.swap(tempClient);
        sharedHttpData->m_clntSocket = sharedClientSocket;
        sharedHttpData->epoll_fd = serverSocket.m_epollfd;

        addfd(serverSocket.m_epollfd,sharedClientSocket->m_clntfd,DEFAULT_EVENTS,sharedHttpData);
    
        //WARNING：定时器初始化
        m_timerManager.addTimer(sharedHttpData,TimerManager::DEFAULT_TIME_OUT);
    }

}

std::vector<std::shared_ptr<HttpData>>
        Epoll::poll(const ServerSocket &serverSocket,int max_event,int timeout)
{
    int event_nums = epoll_wait(serverSocket.m_epollfd,events,max_event,timeout);
    if(event_nums < 0){
        std::cout<<"epoll_num = "<<event_nums<<std::endl;
        std::cout<<"epoll_wait error "<<errno<<std::endl;
        exit(-1);
    }

    std::vector<std::shared_ptr<HttpData>> httpDatas;

    //主线程只对读入信息作处理
    for(int i =0;i < event_nums;i++)
    {
        int fd = events[i].data.fd;

        //监听句柄
        if(fd == serverSocket.m_listenfd)
        {
            handleConnection(serverSocket);
        }
        //出错的描述符，移除定时器，关闭文件描述符
        else if(events[i].events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) 
        {
            auto iter = httpDataMap.find(fd);
            if(iter != httpDataMap.end())
            {
                iter->second->closeTimer();
                ::close(fd);
            }    
        }
        else if(events[i].events & EPOLLIN)
        {
            auto iter = httpDataMap.find(fd);
            if(iter != httpDataMap.end())
            {
                httpDatas.push_back(iter->second);
                //WARNING:清理定时器？原因未知
                iter->second->closeTimer();
                //WARNING:清理fd->httpData映射，原因未知
                httpDataMap.erase(iter);
            }
            else
            {
                std::cout<<"长连接第二次连接没找到"<<std::endl;
                ::close(fd);
            }
            
        }
    }
    return httpDatas;
}
