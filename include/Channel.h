#pragma once 

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

class Channel : noncopyable
{
public:
    int fd() const { return fd_; }          //返回文件描述符

    int events() const { return events_; }    //返回文件描述符对应的事件

    void set_revents(int revt) { revents_ = revt; } //设置文件描述符实际发生的事件

    EventLoop *ownerLoop() {return loop_;} //返回所属的EventLoop，循环,也就是所属的线程


private:
    EventLoop *loop_;   //所属的EventLoop，循环
    int fd_;            //文件描述符
    int events_;        //文件描述符对应的事件
    int revents_;       //文件描述符实际发生的事件  
};
