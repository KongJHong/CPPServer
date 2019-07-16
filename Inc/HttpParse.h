/**
 * 从状态机：负责解析HTTP请求字段
 */

#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

#define CR '\r'
#define LF '\n'
#define LINE_END '\0'
#define PASS

class HttpRequest;

std::ostream &operator<<(std::ostream&,const HttpRequest &);

/* 有限状态机分析HTTP头 静态类*/
class HttpRequestParser
{
public:
    
    enum LINE_STATE{
        LINE_OK = 0,LINE_BAD,LINE_MORE
    };

    enum PARSE_STATE{
        PARSE_REQUESTLINE=0,    //请求首行
        PARSE_HEADER,           //请求头信息
        PARSE_BODY              //请求体（GET没有，只有POST才有）
    };

    enum HTTP_CODE{
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        FORBIDDEN_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    /**
     * 解析一行内容, buffer[checked_index, read_index)
     * check_index是需要分析的第一个字符， read_index已经读取数据末尾下一个字符
     * */
    static LINE_STATE parse_line(char *buffer,int &checked_index,int &read_index);

    /* 解析请求首行 */
    static HTTP_CODE parse_requestline(char *line,PARSE_STATE &parse_state,HttpRequest &request);

    /* 解析请求头部信息 */
    static HTTP_CODE parse_headers(char *line,PARSE_STATE &parse_state,HttpRequest &request);

    /* 解析请求体 */
    static HTTP_CODE parse_body(char *body,HttpRequest &request);

    /**
     * http 请求入口 
     * buffer       : 待处理信息
     * check_index  : 需要分析的第一个字符位置
     * read_index   : 已经读取数据末尾下一个字符
     * start_index   : 开始行
     * request      : 接收数据返回
     * */
    static HTTP_CODE
        parse_content(char *buffer,
                        int &check_index,
                        int &read_index,
                        PARSE_STATE &parse_state,
                        int &start_index,
                        HttpRequest &request);

};


