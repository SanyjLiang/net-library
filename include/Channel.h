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

    // 设置fd相应的事件状态 相当于epoll_ctl add delete
    void enableReading() {events_ |=kReadEvent;update();}
    void disableReading() {events_&=~kReadEvent;update();}
    void enableWriting() {events_ |=kWriteEvent;update();}
    void disableWriting() {events_ &=~kWriteEvent;update();}
    void disableAll() {events_ = kNoneEvent;update();}

    //返回fd当前的事件状态，判断是否要进行删除，增加或者修改
    bool isNoneEvent() const {return events_ == kNoneEvent;}
    bool isReading() const {return events_ & kReadEvent;}
    bool isWriting() const {return events_ & kWriteEvent;}
    EventLoop *ownerLoop() {return loop_;} //返回所属的EventLoop，循环,也就是所属的线程
    void remove();

private:
    void update();

    // 事件类型
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;   //所属的EventLoop，循环
    int fd_;            //文件描述符
    int events_;        //文件描述符对应的事件
    int revents_;       //文件描述符实际发生的事件  
};