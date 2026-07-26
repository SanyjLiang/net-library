#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

class Channel : noncopyable
{
public:
    // 设置事件回调函数类型
    using EventCallback = std::function<void()>;              // 写，关闭，错误事件通常对时间没有要求，所以这里没有时间参数
    using ReadEventCallback = std::function<void(Timestamp)>; // 读事件通常对时间有要求，所以这里有时间参数

    Channel(EventLoop *loop,int fd);
    ~Channel();

    //fd得到Poller通知以后 处理事件 handleEvent在EventLoop::loop()中调用
    void handleEvent(Timestamp receiveTime);
    // 设置事件回调函数
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止当channel被手动remove掉 channel还在执行回调操作
    void tie(const std::shared_ptr<void> &obj);
    int fd() const { return fd_; } // 返回文件描述符

    int events() const { return events_; } // 返回文件描述符对应的事件

    void set_revents(int revt) { revents_ = revt; } // 设置文件描述符实际发生的事件

    // 设置fd相应的事件状态 相当于epoll_ctl add delete
    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    // 返回fd当前的事件状态，判断是否要进行删除，增加或者修改
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
   
    int index() {return index_;}  //返回在Poller中的索引状态

    void set_index(int idx) { index_ = idx; }  // 设置在Poller中的索引状态
    EventLoop *ownerLoop() { return loop_; } // 返回所属的EventLoop，循环,也就是所属的线程
    void remove();

private:
    void update();

    // 事件类型
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_; // 所属的EventLoop，循环
    const int fd_;          // 文件描述符
    int events_;      // 文件描述符对应的事件
    int revents_;     // 文件描述符实际发生的事件
    int index_;       // 在 Poller 中的索引状态,用于快速知道fd的事件状态，比如是否在Poller中注册了

    std::weak_ptr<void>tie_;    //观测 TcpConnection 的生命周期，但不影响它的销毁
    bool tied_;
    // 因为channel通道里可获知fd最终发生的具体的事件events，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_; // 读事件回调函数
    EventCallback writeCallback_;    // 写事件回调函数
    EventCallback closeCallback_;    // 关闭事件回调函数
    EventCallback errorCallback_;    // 错误事件回调函数
};