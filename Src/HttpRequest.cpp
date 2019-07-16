#include "HttpRequest.h"

std::ostream &operator<<(std::ostream &os,const HttpRequest &request)
{
    os<<"method: "<<request.m_method<<std::endl;
    os<<"uri: "<<request.m_uri<<std::endl;
    os<<"version: "<<request.m_version<<std::endl;

    for(auto iter = request.m_headers.begin();iter != request.m_headers.end();iter++)
    {
        os<<iter->first<<":"<<iter->second<<std::endl;
    }
    return os;
}