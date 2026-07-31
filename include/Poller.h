#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

class Poller
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);

    // 虚析构函数，防止子类在堆区开辟内存时，子类析构函数无法调用
    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    //poll(timeoutMs, activeChannels)阻塞等待一段时间
    //把发生了事件的 Channel 存到 activeChannels 里返回
    //返回值是事件发生的时间戳。
    
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    // 判断参数channel是否在前当的Poller当中
    bool hasChannel(Channel *channel) const;

    // EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    // map的key:sockfd value:sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel *>;

    // channels_和Channel类中的index_的区别
    // 1. Channel类中的index_是判断是否在epoll树中注册过的标志位
    //    并判断是否需要ADD/MOD/DEL
    // 2.channels_是用来快速找到和管理对应的Channel
    //    Poller 通过 Channel 对象来管理 epoll 树上的所有 fd！
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义Poller所属的事件循环EventLoop
};