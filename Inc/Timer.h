/**
 * 定时器：用于保存长连接和其他定时功能
 * 主要数据结构是堆数组
 * 由定时器节点类，通过定时器管理（堆）类管理
 */

#pragma once
#include <queue>
#include <deque>
#include "HttpData.h"
#include "Utils.h"

class HttpData;

//定时器节点类,负责管理节点，包括每个节点自己的时间，保存的数据指针
class TimerNode{
public:
    explicit TimerNode(std::shared_ptr<HttpData> httpData,size_t timeout);
    ~TimerNode();

public:
    /* 该节点是否已经被超时删除 */
    bool isDeleted() const {return m_deleted;}

    /* 返回超时时间，即初始设定值,相对时间 */
    size_t getExpireTime() const {return m_expiredTime_ms;}

    /* 是否超时 */
    bool isExpire(){
        return m_expiredTime_ms < m_currentTime_ms;
    }

    /**
     *  删除节点时调用，用于清理自身的堆内存
     *  删除采用标记删除，并及时析构HttpData
     *  关闭定时器时应该把httpDataMap里的HttpData一起erase
    */
    void deleted();

    //获取当前时间
    static void current_time();

    //当前时间，绝对时间，static，单位：ms
    static size_t m_currentTime_ms;

    void (*cb_func)(std::shared_ptr<HttpData>);

private:
    //告诉容器是否要删除
    bool m_deleted;       

    //超时时间,绝对时间，单位：毫秒      
    size_t  m_expiredTime_ms;   

    // 保存的httpData信息
    std::shared_ptr<HttpData> m_httpData;
};

////////////////////////////////////////////////////////////////////
struct TimerCmp{
    bool operator()(std::shared_ptr<TimerNode> &a,std::shared_ptr<TimerNode> &b){
            return a->getExpireTime() > b->getExpireTime();
    }
};

//定时器管理类：用于管理定时器，用堆管理定时器
class TimerManager{
public:
    using shared_TimerNode = std::shared_ptr<TimerNode>;

public:
    /* 定时器加入管理队列 */
    void addTimer(std::shared_ptr<HttpData> httpData,size_t timeout);

    /**
     * 处理定时器过期事件：调整堆队列 
     * WARNING：没有对定时事件进行处理
     * */
    void handle_expired_event();

    /* 默认定时器过期时间 */
    const static size_t DEFAULT_TIME_OUT;

private:
   // typedef std::shared_ptr<TimerNode> shared_TimerNode;

    std::priority_queue<shared_TimerNode,std::deque<shared_TimerNode>,TimerCmp> m_timerQueue;
    
    /* 保护定时器队列的锁 */
    MutexLock m_queueLock;

};



