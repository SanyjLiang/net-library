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

    Poller(EventLoop *loop);

protected:
    // map的key:sockfd value:sockfd所属的channel通道类型
    using ChannelMap=std::unordered_map<int,Channel*>;

    //channels_和Channel类中的index_的区别
    //1. Channel类中的index_是判断是否在epoll树中注册过的标志位
    //   并判断是否需要ADD/MOD/DEL
    //2.channels_是用来快速找到和管理对应的Channel
    //   Poller 通过 Channel 对象来管理 epoll 树上的所有 fd！
    ChannelMap channels_;
private:
    EventLoop *loop_; // 定义Poller所属的事件循环EventLoop

}