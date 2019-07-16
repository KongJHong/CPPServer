#pragma once

#include <string>
#include <unordered_map>

#include "HttpRequest.h"

/**
 * 在访问网页时，MIME type帮助浏览器识别一个HTTP请求返回
 * 的是什么内容的数据，应该如何打开、如何显示。
 */
struct MimeType{
    MimeType(const std::string &str):type(str){}

    MimeType(const char *str):type(str){}

    std::string type;
};

//在.cpp中
extern std::unordered_map<std::string,MimeType> Mime_map;


class HttpResponse
{
public:
    //访问返回头——键
    enum HttpStatusCode{
        Unknow,
        k200OK = 200,
        k403forbidden = 403,
        k404NotFound = 404
    };

    explicit HttpResponse(bool mKeep = true)
                :m_statusCode(Unknow),m_keep_alive(mKeep),m_mime("text/html"),
                m_body(nullptr),m_version(HttpRequest::HTTP_10){}


    void setStatusCode(HttpStatusCode code){
        m_statusCode = code;
    }

    HttpStatusCode statuesCode() const {
        return m_statusCode;
    }

    void setBody(const char *buf){
        m_body = buf;
    }

    //在输入前准备好，所以是直接设置，而不是累加
    void setContentLength(int len){
        m_contentLength = len;
    }

    void setVersion(const HttpRequest::HTTP_VERSION &version){
        m_version = version;
    }

    const HttpRequest::HTTP_VERSION version() const{
        return m_version;
    }

    void setStatusMsg(const std::string &msg){
        m_statusMsg = msg;
    }

    const std::string &statusMsg()const {
        return m_statusMsg;
    }

    void setFilePath(const std::string &path){
        m_filePath = path;
    }

    const std::string &filePath() const{
        return m_filePath;
    }

    void setMime(const MimeType &mime){
        m_mime = mime;
    }

    void setKeepAlive(bool isAlive){
        m_keep_alive = isAlive;
    }

    bool keep_alive() const{
        return m_keep_alive;
    }

    /* 增加返回HTTP协议头 */
    void addHeader(const std::string &key,const std::string &value){
        m_headers[key] = value;
    }

    /* 返回协议内容组成 */
    void constructBuffer(char*)const;

    ~HttpResponse(){
        if(m_body != nullptr)
            delete[] m_body;
    }

private:
    /* 访问返回头——键 默认Unknow*/
    HttpStatusCode m_statusCode;

    /* 访问返回头——值 */
    std::string m_statusMsg;

    /* 返回HTTP版本号 默认HTTP1.1 */
    HttpRequest::HTTP_VERSION m_version;

    /* 是否保持连接 默认true*/
    bool m_keep_alive;

    /* Mime类型提示 默认text/html*/
    MimeType m_mime;
    
    /* 返回体 默认为nullptr*/
    const char *m_body;

    /* 返回体长度 */
    int m_contentLength;

    /* 返回的文件的路径 */
    std::string m_filePath;

    /* 辅助变量——返回头的键值匹配 */
    std::unordered_map<std::string,std::string> m_headers;
};