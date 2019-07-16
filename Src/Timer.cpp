#include "Timer.h"
#include "Epoll.h"
#include <sys/time.h>

size_t TimerNode::m_currentTime_ms = 0; //当前时间初始化

const size_t TimerManager::DEFAULT_TIME_OUT = TIME_OUT * 1000; //默认长连接120s

/*********************************************************/
//                                                       //
//                      TimerNode类                      //
//                                                       //
/*********************************************************/

void inline TimerNode::current_time(){
    struct timeval cur;
    //利用 sys/time.h文件中的gettimeofday函数返回当前时间
    gettimeofday(&cur,NULL);
    //精确到ms
    m_currentTime_ms = (cur.tv_sec * 1000) + (cur.tv_usec / 1000);
}

TimerNode::TimerNode(std::shared_ptr<HttpData> httpData,size_t timeout):
                m_deleted(false),
                m_httpData(httpData)
{
    current_time();//获取当前系统时间
    m_expiredTime_ms = m_currentTime_ms + timeout;
}

void TimerNode::deleted()
{
    m_httpData.reset();
    m_deleted = true;
}

TimerNode::~TimerNode(){
    //析构时如果是被deleted，则httpData为NULL，不处理
    //如果是超时，则需要把Epoll中的httpDataMap对应fd进行删除
    if(m_httpData){
        auto iter = Epoll::httpDataMap.find(m_httpData->m_clntSocket->m_clntfd);
        if(iter != Epoll::httpDataMap.end()){
            Epoll::httpDataMap.erase(iter);
        }
    }
}


/*********************************************************/
//                                                       //
//                    TimerManager类                      //
//                                                       //
/*********************************************************/



void TimerManager::addTimer(std::shared_ptr<HttpData> httpData,size_t timeout)
{
    shared_TimerNode timerNode(new TimerNode(httpData,timeout));

    m_queueLock.lock();
        
    m_timerQueue.push(timerNode);

    /* 将Timer和httpData关联起来 */
    httpData->setTimer(timerNode);

    m_queueLock.unlock();
    
}


void TimerManager::handle_expired_event()
{
    m_queueLock.lock();

    //更新时间
    TimerNode::current_time();
    //std::cout << "开始处理超时事件" << std::endl;
    while(!m_timerQueue.empty()){
        shared_TimerNode timerNode = m_timerQueue.top();
        if(timerNode->isDeleted()){
            // 删除节点
            m_timerQueue.pop();
        }else if(timerNode->isExpire()){
            // 过期，删除节点
            m_timerQueue.pop();
        }else{
            break;
        }
    }

    m_queueLock.unlock();
}

