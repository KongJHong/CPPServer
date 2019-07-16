#include "HttpResponse.h"


std::unordered_map<std::string,MimeType> Mime_map = {
        {".html","text/html"},
        {".xml","text/xml"},
        {".xhtml","application/xhtml+xml"},
        {".txt","text/plain"},
        {".rtf","application/rtf"},
        {".pdf","application/pdf"},
        {".word","application/msword"},
        {".png","image/png"},
        {".gif","image/gif"},
        {".jpg","image/jpeg"},
        {".jpeg","image/jpeg"},
        {".au","audio/basic"},
        {".mpeg","video/mpeg"},
        {".mpg","video/mpeg"},
        {".avi","video/x-msvideo"},
        {".gz","application/x-gzip"},
        {".tar","application/x-tar"},
        {".css","text/css"},
        {"","text/plain"},      //默认
        {"default","text/plain"}
};


void HttpResponse::constructBuffer(char* buffer) const
{
    //版本
    if(m_version == HttpRequest::HTTP_10)
    {
        sprintf(buffer,"HTTP/1.0 %d %s\r\n",m_statusCode,m_statusMsg.c_str());
    }
    else
    {
        sprintf(buffer,"HTTP/1.1 %d %s\r\n",m_statusCode,m_statusMsg.c_str());
    }

    //头部字段
    for(auto iter = m_headers.begin();iter != m_headers.end();iter++){
        sprintf(buffer,"%s%s: %s\r\n",buffer,iter->first.c_str(),iter->second.c_str());
    }

    //keep-alive
    if(m_keep_alive)
    {
        sprintf(buffer,"%sConnection:keep-alive\r\n",buffer);
    }
    else
    {
        sprintf(buffer,"%sConnection:close\r\n",buffer);
    }
}