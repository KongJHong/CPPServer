#include "CommonHead.h"
#include "Utils.h"



int setnonblocking(int fd)
{
    __uint32_t old_option = fcntl(fd,F_GETFL);
    __uint32_t new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

bool check_base_path(char *basePath)
{
    struct stat file;
    if(stat(basePath,&file) == -1)//返回文件属性
        return false;
    
    //不是目录，或者不可访问
    if(!S_ISDIR(file.st_mode) || access(basePath,R_OK) == -1)
        return false;

    return true;
}

bool setReusePort(int fd)
{
    int reuse = 1;
    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(void*)&reuse,sizeof(reuse))==-1)
    {
        return false;
    }
    
    return true;
}

std::string &ltrim(std::string &str)
{
    if(str.empty()){
        return str;
    }

    str.erase(0,str.find_first_not_of(" \t"));
    return str;
}


std::string &rtrim(std::string &str)
{
    if(str.empty()){
        return str;
    }

    str.erase(str.find_last_not_of(" \t")+1);
    return str;
}


std::string &trim(std::string &str)
{
    if(str.empty())return str;

    ltrim(str);
    rtrim(str);

    return str;
}