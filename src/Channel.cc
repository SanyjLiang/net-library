#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel:: kNoneEvent=0;         //空事件   
const int Channel:: kReadEvent=EPOLLIN|EPOLLPRI;      //读事件  0000 0001 | 0000 0010
const int Channel:: kWriteEvent=EPOLLOUT;             //写事件  0000 0100


Channel::Channel(EventLoop *loop,int fd)
        :loop_(loop)
        ,fd_(fd)
        ,events_(0)
        ,revents_(0)
        ,index_(-1)
        ,tied_(false)
{

}

Channel::~Channel()
{
    
}

// channel的tie方法什么时候调用过?  TcpConnection => channel
/**
 * TcpConnection中注册了Channel对应的回调函数，传入的回调函数均为TcpConnection
 * 对象的成员方法，因此可以说明一点就是：Channel的结束一定晚于TcpConnection对象！
 * 此处用tie去解决TcpConnection和Channel的生命周期时长问题，从而保证了Channel对象能够在
 * TcpConnection销毁前销毁。
 **/


//参数obj:通常是 TcpConnection 通过 shared_from_this() 获取的 shared_ptr
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_=obj;       //让弱指针tie知道自己应该观测共享指针obj所管理的内存
    tied_=true;     //设置tied_为true，表示弱指针已经有观测的内存了
}
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

void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {
        //获取一个shared_ptr实例
        std::shared_ptr<void>guard=tie_.lock();

        //如果shared_ptr实例存在，说明TcpConnection内存存在，可以调用回调函数
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    } 

}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    
    //关闭连接
    //revents_ & EPOLLHUP  判断是否发生挂断事件
    //！（revents_ & EPOLLIN）  判断是否没有读事件
    //如果发生挂断事件且没有读事件，说明连接被关闭了
    if((revents_&EPOLLHUP)&&!(revents_&EPOLLIN))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }

    //错误
    if(revents_&EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }

    //读
    if(revents_&(EPOLLIN|EPOLLPRI))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    //写
    if(revents_&EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}