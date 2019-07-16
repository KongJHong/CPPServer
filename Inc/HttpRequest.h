/**
 * 主状态机，定义变量，从状态机处理在HttpParse.cpp 内
 */
#pragma once

#include <iostream>
#include <unordered_map>
#include <string>

class HttpRequest;


std::ostream &operator<<(std::ostream&,const HttpRequest&);

struct HttpRequest{
    /* 重载HttpRequest <<  */
    friend std::ostream &operator<<(std::ostream&,const HttpRequest&);

    enum HTTP_VERSION{
        HTTP_10 = 0,HTTP_11,VERSION_NOT_SUPPORT
    };

    enum HTTP_METHOD{
        GET = 0,POST,PUT,DELETE,METHOD_NOT_SUPPORT
    };

    enum HTTP_HEADER{   //映射键
        Host = 0,
        User_Agent,
        Connection,         //保存Keep-Alive
        Accept_Encoding,
        Accept_Language,
        Accept,
        Cache_Control,
        Upgrade_Insecure_Requests
    };


    /* unordered_map的自定义hash排序 */
    struct EnumClassHash{
        template<typename T>
        std::size_t operator()(T t) const{
            return static_cast<std::size_t>(t);
        }
    };

    HttpRequest(std::string url = std::string(""),HTTP_METHOD method = METHOD_NOT_SUPPORT,
                HTTP_VERSION version = VERSION_NOT_SUPPORT):
                m_method(method),m_version(version),m_uri(url),m_content(nullptr),
                m_headers(std::unordered_map<HTTP_HEADER,std::string,EnumClassHash>())
    {}

    /* 请求方式 */
    HTTP_METHOD m_method;
    
    /* HTTP版本号 */
    HTTP_VERSION m_version;
    
    /* 请求的Uri */
    std::string m_uri;
    
    /* 返回的内容 */
    char *m_content;

    /* 已经保存的了的请求头部信息映射 */
    std::unordered_map<HTTP_HEADER,std::string,EnumClassHash> m_headers;

    /* 辅助变量——名称映射 在HttpParse中定义*/
    static std::unordered_map<std::string,HTTP_HEADER> header_map;

};