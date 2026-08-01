//#define MUDEBUG

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"


const int kNew = -1;    // 从未加入Poller：channels_无记录 + epoll树中未注册（Channel初始状态）    // channel的成员index_初始化为-1
const int kAdded = 1;   // 正常监听中：channels_有记录 + epoll树中已注册
const int kDeleted = 2; // 暂停监听：channels_仍有记录，但从epoll树中临时摘除（可随时重新启用）

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) // vector<epoll_event>(16)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        LOG_INFO("%d events happend\n", numEvents); // LOG_DEBUG最合理
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() error!");
        }
    }

    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel; // 将channel添加至Poller中
        }
        else // index == kDeleted
        {
        }
        channel->set_index(kAdded);     // 如果是新添加的channel 则设置为kAdded状态
        update(EPOLL_CTL_ADD, channel); // 将channel添加至epoll树中
    }
    else // channel已经在Poller中注册过了
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            // ================== 临时删除（暂停监听用）==================
            // 触发场景：disableAll()后无关注事件，后续还要复用（如TCP写缓冲区满、空闲暂停读）
            // 做了什么：✅ 从内核epoll树摘fd | ❌ channels_不删记录 | ❌ fd不关闭（OS不回收）
            // index改完：设为kDeleted，后续enableXXX()直接ADD重新上树，无需重新插map
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else // channel的事件状态不为空 则修改epoll树中的事件状态
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// ================== 彻底删除（销毁前清理用）==================
// 触发场景：TcpConnection断开、Channel要析构，再也不用这个Channel了
// 做了什么：✅ index=kAdded时才从epoll树摘fd | ✅ channels_必erase(fd)删记录 | ❌ 本函数不close(fd)（上层TcpConnection析构时才关）
// index改完：重置为kNew（初始状态），下次再加当全新Channel处理
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->set_index(kNew);
}

// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}

void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();

    event.events = channel->events();
    // 这里是联合体所以data.fd写了没有意义会被data.ptr覆盖
    //  event.data.fd = fd;
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}