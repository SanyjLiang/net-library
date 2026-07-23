#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel:: kNoneEvent=0;         //空事件
const int Channel:: kReadEvent=EPOLLIN|EPOLLPRI;      //读事件
const int Channel:: kWriteEvent=EPOLLOUT;             //写事件   


//update 和remove => EpollPoller 更新channel在poller中的状态
/**
 * 当改变channel所表示的fd的events事件后，update负责再poller里面更改fd相应的事件epoll_ctl
 **/
void Channel::update()
{
    // 通过channel所属的eventloop，调用poller的相应方法，注册fd的events事件
    loop_->updateChannel(this);
}

void Channel::remove()
{
    // 在channel所属的EventLoop中把当前的channel删除掉
    loop_->removeChannel(this);
}
