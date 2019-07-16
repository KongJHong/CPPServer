#include "HttpServer.h"
#include "ThreadPool.h"
#include "Utils.h"
#include "Epoll.h"
#include "HttpParse.h"


#include <string>
#include <iostream>
#include <sstream>

char NOT_FOUND_PAGE[] = "<html>\n"
                        "<head><title>404 Not Found</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>404 Not Found</h1></center>\n"
                        "<hr><center>LC WebServer/0.3 (Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char FORBIDDEN_PAGE[] = "<html>\n"
                        "<head><title>403 Forbidden</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>403 Forbidden</h1></center>\n"
                        "<hr><center>LC WebServer/0.3 (Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char INDEX_PAGE[] = "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "    <title>Welcome to LC WebServer!</title>\n"
                    "    <style>\n"
                    "        body {\n"
                    "            width: 35em;\n"
                    "            margin: 0 auto;\n"
                    "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
                    "        }\n"
                    "    </style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<h1>Welcome to LC WebServer!</h1>\n"
                    "<p>If you see this page, the lc webserver is successfully installed and\n"
                    "    working. </p>\n"
                    "\n"
                    "<p>For online documentation and support please refer to\n"
                    "    <a href=\"https://github.com/KongJHong\">KongJHong</a>.<br/>\n"
                    "\n"
                    "<p><em>Thank you for using KongJHong WebServer.</em></p>\n"
                    "</body>\n"
                    "</html>";

char TEST[] = "HELLO WORLD";


//定义在main.cpp中
extern std::string basePath;

void HttpServer::run(int thread_num,int max_request)
{
    /* 初始化线程池 */
    ThreadPool threadPool(thread_num,max_request);

    /* 初始化epoll */
    int epoll_fd = Epoll::init(MAX_EVENT_NUMBER);

    shared_httpData httpData(new HttpData());
    httpData->epoll_fd = epoll_fd;

    /* servSocket保存m_epollfd */
    servSocket.m_epollfd = epoll_fd;

    __uint32_t event = (EPOLLIN | EPOLLET);
    Epoll::addfd(epoll_fd,servSocket.m_listenfd,event,httpData);

    while(true)
    {
        std::vector<shared_httpData> events = Epoll::poll(servSocket,MAX_EVENT_NUMBER,-1);

        for(auto &req : events){
            threadPool.append(req,std::bind(&HttpServer::do_request,this,std::placeholders::_1));
        }

        //处理定时器超时事件
        Epoll::m_timerManager.handle_expired_event();
    }

}

void HttpServer::do_request(std::shared_ptr<void> arg)
{
    //智能指针类型转换
    shared_httpData sharedHttpData = std::static_pointer_cast<HttpData>(arg);

    char buffer[BUFFER_SIZE];
    memset(buffer,0,BUFFER_SIZE);
    
    int check_index = 0,read_index = 0,start_index = 0;
    ssize_t recv_data;
    HttpRequestParser::PARSE_STATE parse_state = HttpRequestParser::PARSE_REQUESTLINE;

    while(true){
        recv_data = recv(sharedHttpData->m_clntSocket->m_clntfd,buffer+read_index,BUFFER_SIZE - read_index,0);
        if(recv_data == -1){
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
               // std::cout<<"recv_data == -1 && errno == "<<errno<<std::endl;
                break;
            }
            std::cout<<"reading faild"<<std::endl;
            return;
        }

        if(recv_data == 0){
            std::cout<<"Connection closed by peer"<<std::endl;
            break;
        }

        read_index += recv_data;

        //解析HTTP头
        HttpRequestParser::HTTP_CODE retcode = HttpRequestParser::parse_content(
                        buffer,check_index,read_index,parse_state,start_index,*sharedHttpData->m_request);
    
    
        if(retcode == HttpRequestParser::NO_REQUEST){
            continue;
        }
    
        if(retcode == HttpRequestParser::GET_REQUEST){
            // 检查keep_alive选项
            auto iter = sharedHttpData->m_request->m_headers.find(HttpRequest::Connection);
            if(iter != sharedHttpData->m_request->m_headers.end()){
                if(iter->second == "keep-alive" )
                {
                   // std::cout<<"设置keep-alive"<<std::endl;
                    sharedHttpData->m_response->setKeepAlive(true);
                    //timeout = 120
                    std::ostringstream osstr;
                    osstr<<"timeout="<<TIME_OUT;
                    sharedHttpData->m_response->addHeader("Keep-Alive",osstr.str());
                }
                else
                {
                   // std::cout<<"不设置keep-alive"<<std::endl;
                    sharedHttpData->m_response->setKeepAlive(false);
                }
            }

            header(sharedHttpData);
            getMime(sharedHttpData);

            //返回CGI文件
            FileState fileState =static_file(sharedHttpData,basePath.c_str());
            send(sharedHttpData,fileState);

            //如果是keep-alive else sharedHttpData将会自动解析释放clientSocket，从而关闭资源
            if(sharedHttpData->m_response->keep_alive()){
                //再次添加定时器
                Epoll::modfd(sharedHttpData->epoll_fd,sharedHttpData->m_clntSocket->m_clntfd,Epoll::DEFAULT_EVENTS,sharedHttpData);
                Epoll::m_timerManager.addTimer(sharedHttpData,TimerManager::DEFAULT_TIME_OUT);
            }
        }else{
            std::cout<<"Bad Request"<<std::endl;
        }
    }  
}

void HttpServer::header(shared_httpData httpData)
{
    if(httpData->m_request->HTTP_10){
        httpData->m_response->setVersion(HttpRequest::HTTP_10);
    }else{
        httpData->m_response->setVersion(HttpRequest::HTTP_11);
    }
    httpData->m_response->addHeader("Server",DEFAULT_SERVER_STRING);
}


HttpServer::FileState HttpServer::static_file(shared_httpData httpData,const char *basepath)
{
    struct stat file_stat;
    char *file = new char[strlen(basepath) + strlen(httpData->m_response->filePath().c_str()) + 1];
    strcpy(file,basepath);
    strcat(file,httpData->m_response->filePath().c_str());

    // 文件不存在
    if(httpData->m_response->filePath() == "/" || stat(file,&file_stat) < 0){
        httpData->m_response->setMime(MimeType("text/html"));
        if(httpData->m_response->filePath() == "/"){
            httpData->m_response->setStatusCode(HttpResponse::k200OK);
            httpData->m_response->setStatusMsg("OK");
        }else{
            httpData->m_response->setStatusCode(HttpResponse::k404NotFound);
            httpData->m_response->setStatusMsg("Not Found");
        }
        return FILE_NOT_FOUND;
    }

    //不是普通文件或无访问权限
    if(!S_ISREG(file_stat.st_mode)){
        //设置MIME为html
        httpData->m_response->setMime(MimeType("text/html"));
        httpData->m_response->setStatusCode(HttpResponse::k403forbidden);
        httpData->m_response->setStatusMsg("ForBidden");

        std::cout<<"not normal file"<<std::endl;
        return FILE_FORBIDDEN;
    }

    httpData->m_response->setStatusCode(HttpResponse::k200OK);
    httpData->m_response->setStatusMsg("OK");
    httpData->m_response->setFilePath(file);


    delete []file;
    return FILE_OK;


}

void HttpServer::getMime(std::shared_ptr<HttpData> httpData)
{
    std::string filepath = httpData->m_request->m_uri;
    std::string mime;

    int pos;

    //WARNING：暂时将参数丢掉，后面再搞
    if((pos = filepath.rfind('?')) != std::string::npos){
        filepath.erase(filepath.rfind('?'));
    }
    
    //找到后缀名赋值给mime
    if(filepath.rfind('.')!= std::string::npos){
        mime = filepath.substr(filepath.rfind('.'));
    }

    decltype(Mime_map)::iterator iter = Mime_map.find(mime);
    if(iter != Mime_map.end()){//
        httpData->m_response->setMime(iter->second);
    }
    else{
        httpData->m_response->setMime(Mime_map.find("default")->second);
    }

    httpData->m_response->setFilePath(filepath);
}


void HttpServer::send(shared_httpData httpData,FileState fileState)
{
    char header[BUFFER_SIZE];
    memset(header,'\0',BUFFER_SIZE);
    const char* internal_error = "Internal Error";
    struct stat file_stat;
    httpData->m_response->constructBuffer(header);

    //404
    if(fileState == FILE_NOT_FOUND){
        //如果是'/'就发送默认页
        if(httpData->m_response->filePath() == std::string("/")){
            // 构建返回包
            sprintf(header,"%sContent-length: %ld\r\n\r\n",header,strlen(INDEX_PAGE));
            sprintf(header,"%s%s",header,INDEX_PAGE);
        }
        else{
            sprintf(header,"%sContent-length: %ld\r\n\r\n",header,strlen(NOT_FOUND_PAGE));
            sprintf(header,"%s%s",header,NOT_FOUND_PAGE);
        }

        ::send(httpData->m_clntSocket->m_clntfd,header,strlen(header),0);
        return;
    }

    //403
    if(fileState == FILE_FORBIDDEN){
        sprintf(header,"%sContent-length: %ld\r\n\r\n",header,strlen(FORBIDDEN_PAGE));
        sprintf(header,"%s%s",header,FORBIDDEN_PAGE);
        ::send(httpData->m_clntSocket->m_clntfd,header,strlen(header),0);
        return;
    }


    //200
    if(stat(httpData->m_response->filePath().c_str(),&file_stat) < 0){
        sprintf(header,"%sContent-length: %ld\r\n\r\n",header,strlen(internal_error));
        sprintf(header,"%s%s",header,internal_error);
        ::send(httpData->m_clntSocket->m_clntfd,header,strlen(header),0);
        return;
    }

    int filefd = ::open(httpData->m_response->filePath().c_str(),O_RDONLY);
    //打开文件失败
    if(filefd < 0){
        std::cout<<"打开文件失败"<<std::endl;
        sprintf(header,"%sContent-length: %ld\r\n\r\n",header,strlen(internal_error));
        sprintf(header,"%s%s",header,internal_error);
        ::send(httpData->m_clntSocket->m_clntfd,header,strlen(header),0);
        return;
    }

    //发送头
    sprintf(header,"%sContent-length: %ld\r\n\r\n",header,file_stat.st_size);
    ::send(httpData->m_clntSocket->m_clntfd,header,strlen(header),0);
    //发送文件内容(通过共享内存)
    /* 
    void *mapbuf = mmap(NULL,file_stat.st_size,PROT_READ,MAP_PRIVATE,filefd,0);
    ::send(httpData->m_clntSocket->m_clntfd,mapbuf,file_stat.st_size,0);
    munmap(mapbuf,file_stat.st_size);
    */
    //直接用sendfile函数
    sendfile(httpData->m_clntSocket->m_clntfd,filefd,NULL,file_stat.st_size);

    close(filefd);
    return;

/*
err:
    sprintf(header,"%sContent-length: %d\r\n\r\n",header,strlen(internal_error));
    sprintf(header,"%s%s",header,internal_error);
    ::send(httpData->m_clntSocket->m_clntfd,header,strlen(header),0);
    return;
*/
}


