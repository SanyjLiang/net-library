#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop *loop)
    : loop_(loop)
{
}