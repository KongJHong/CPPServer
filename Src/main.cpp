
#include "CommonHead.h"
#include "Utils.h"
#include "DaemonRun.h"
#include "HttpServer.h"

//CGI文件目录
std::string basePath = ".";

//加入epoll内核，默认是ET
void addfd(int epollfd,int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN|EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}


int main(int argc,char *argv[])
{

    int threadNumber    = 4;        //默认线程数
    int port            = DEFAULT_PORT;     //默认端口
    const char *str     = "t:p:r:d";//选项字符 t:thread p:port r:root d:daemon
    int opt;                        //获取选项
    bool daemon         = false;    //开启守护进程
    

    while((opt = getopt(argc,argv,str)) != -1)
    {
        switch(opt)
        {
            case 't':
            {
                threadNumber = atoi(optarg);
                break;
            }
            case 'r':
            {
                if(!check_base_path(optarg))
                {
                    printf("Warning: \"%s\" is not existed or no permission ,\
                                will use current dir as website root dir\n",optarg);
                    char tempPath[256];
                    if(getcwd(tempPath,256) == NULL)
                    {//get current working dir
                        puts("get current working dir error");
                        basePath = '.';
                    }
                    else
                    {
                        basePath = tempPath;
                    }
                    break;
                }
                if(optarg[strlen(optarg)-1] == '/'){
                    optarg[strlen(optarg)-1] = '\0';
                }
                basePath = optarg;
                break;
            }
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            case 'd':
            {
                daemon = true;
                break;
            }

            default:break;
        }
    }

    if(daemon)
        daemon_run();


    //输出配置信息
    {
        printf("**************LC WebServer 配置信息***************\n");
        printf("PID:\t%d\n",getpid());
        printf("端口:\t%d\n",port);
        printf("线程数:\t%d\n",threadNumber);
        printf("根目录:\t%s\n",basePath.c_str());
    }

    signal(SIGPIPE,SIG_IGN);//忽略写入FIN的socket返回

    HttpServer httpServer(port);
    httpServer.run(threadNumber,MAX_EVENT_NUMBER);
   
    return 0;
}