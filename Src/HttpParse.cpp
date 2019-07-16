#include "HttpParse.h"

#include "HttpRequest.h"
#include "Utils.h"
#include <string.h>
#include <algorithm>

std::unordered_map<std::string,HttpRequest::HTTP_HEADER> HttpRequest::header_map = {
    {"HOST",                        HttpRequest::Host},
    {"USER_AGENT",                  HttpRequest::User_Agent},
    {"CONNECTION",                  HttpRequest::Connection},
    {"ACCEPT-ENCODING",             HttpRequest::Accept_Encoding},
    {"ACCEPT-LANGUAGE",             HttpRequest::Accept_Language},
    {"ACCEPT",                      HttpRequest::Accept},
    {"CACHE-CONTROL",               HttpRequest::Cache_Control},
    {"UPGREADE-INSECURE-REQUESTS",  HttpRequest::Upgrade_Insecure_Requests}

};

HttpRequestParser::LINE_STATE 
HttpRequestParser::parse_line(char *buffer,int &checked_index,int &read_index)
{
    char temp;
    for(;checked_index < read_index;checked_index++){
        temp = buffer[checked_index];
        if(temp == CR){
            // 到末尾，需要读入
            if(checked_index + 1 == read_index)
                return LINE_MORE;

            //完整的"\r\n"
            if(buffer[checked_index + 1] == LF) {
                buffer[checked_index++] = LINE_END;
                buffer[checked_index++] = LINE_END;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_MORE;
}


HttpRequestParser::HTTP_CODE 
HttpRequestParser::parse_requestline(char *line,PARSE_STATE &parse_state,HttpRequest &request)
{
    //返回由 空格 或 \t 分割的首个位置
    char *url = strpbrk(line," \t");
    
    if(!url){
        return BAD_REQUEST;
    }

    //分割method和url
    *url++ = '\0';

    char *method = line;

    if(strcasecmp(method,"GET") == 0){
        request.m_method = HttpRequest::GET;
    }else if(strcasecmp(method,"POST") == 0){
        request.m_method = HttpRequest::POST;
    }else if(strcasecmp(method,"PUT") == 0){
        request.m_method = HttpRequest::PUT;
    }else{
        return BAD_REQUEST;
    }

    //跳过所有 空格 或者 \t ，返回的是跳过后的首个下标
    url += strspn(url," \t");
    char *version = strpbrk(url," \t");
    if(!version){
        return BAD_REQUEST;
    }

    *version++ = '\0';
    version += strspn(version," \t");//跳过URI，url保存这uri信息

    // HTTP/1.1 后面可能还存在空白字符
    if (strncasecmp("HTTP/1.1", version, 8) == 0) {
        request.m_version = HttpRequest::HTTP_11;
    } else if (strncasecmp("HTTP/1.0", version, 8) == 0) {
        request.m_version = HttpRequest::HTTP_10;
    } else {
        return BAD_REQUEST;
    }

    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    } else if (strncasecmp(url, "/", 1) == 0) {
        PASS;
    } else {
        return BAD_REQUEST;
    }

    if (!url || *url != '/') {
        return BAD_REQUEST;
    }
    request.m_uri = std::string(url);
    // 分析头部字段
    parse_state = PARSE_HEADER;
    return NO_REQUEST;
}

HttpRequestParser::HTTP_CODE 
HttpRequestParser::parse_headers(char *line,PARSE_STATE &parse_state,HttpRequest &request)
{
    // 句尾
    if(*line == '\0'){
        //不是GET才检查BODY请求体
        if(request.m_method == HttpRequest::GET){
            return GET_REQUEST;
        }
        parse_state = PARSE_BODY;
        return NO_REQUEST;
    }

    char key[100],value[300];

    //利用正则表达式匹配 key,value
    sscanf(line,"%[^:]:%[^:]",key,value);

    decltype(HttpRequest::header_map)::iterator iter;
    std::string key_s(key);
    std::transform(key_s.begin(),key_s.end(),key_s.begin(),::toupper);//转大写
    std::string value_s(value);

    if((iter = HttpRequest::header_map.find(trim(key_s))) != HttpRequest::header_map.end())
    {
        request.m_headers.insert(std::make_pair(iter->second,trim(value_s)));
    }
    else
    {

    }

    return NO_REQUEST;
}


HttpRequestParser::HTTP_CODE 
HttpRequestParser::parse_body(char *body,HttpRequest &request)
{
    strcpy(request.m_content,body);
    return GET_REQUEST;
}


HttpRequestParser::HTTP_CODE 
HttpRequestParser::parse_content(char *buffer,int &check_index,int &read_index,
                         PARSE_STATE &parse_state,int &start_index,HttpRequest &request)
{
    LINE_STATE line_state = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;

    while((line_state = parse_line(buffer,check_index,read_index)) == LINE_OK)
    {
        char *tmp = buffer + start_index;   //这一行在buffer中的起始位置
        start_index = check_index;          //下一个行起始位置

        switch(parse_state){
            case PARSE_REQUESTLINE:{
                retcode = parse_requestline(tmp,parse_state,request);
                if(retcode == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            case PARSE_HEADER:{
                retcode = parse_headers(tmp,parse_state,request);
                if(retcode == BAD_REQUEST)
                    return BAD_REQUEST;
                else if(retcode == GET_REQUEST)
                    return GET_REQUEST;
                break;
            }
            case PARSE_BODY:{
                retcode = parse_body(tmp,request);
                if(retcode == GET_REQUEST)
                    return GET_REQUEST;
                return BAD_REQUEST;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    if(line_state == LINE_MORE){
        //请求头读完了，应该读请求体
        return NO_REQUEST;
    }else{
        return BAD_REQUEST;
    }
}