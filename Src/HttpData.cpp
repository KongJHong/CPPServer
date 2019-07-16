#include "HttpData.h"


void HttpData::closeTimer()
{
    //判断Timer是否存在，有可能已经超时释放了
    if(weak_timer.lock())
    {
        std::shared_ptr<TimerNode> tempTimer(weak_timer.lock());
        tempTimer->deleted();

        //断开weak_ptr
        weak_timer.reset();
    }
}

void HttpData::setTimer(std::shared_ptr<TimerNode> timer)
{
    weak_timer = timer;
}