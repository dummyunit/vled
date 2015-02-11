#ifndef _VLED_EVENT_REACTOR_H_
#define _VLED_EVENT_REACTOR_H_

#include "fd_guard.h"

#include <sys/epoll.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
#include <system_error>

/* Simple event loop built around epoll. */
class event_reactor
{
public:
	enum {
		e_read = EPOLLIN,
		e_write = EPOLLOUT,
	};

	event_reactor();

	template <class Handler>
	void add_fd_watch(int fd, int events, Handler handler);
	void delete_fd_watch(int fd);
	
	void run();

private:
	fd_guard epfd_;
	std::map<int, std::function<void (int, int)> > callbacks_;
};

inline event_reactor::event_reactor()
{
	epfd_.reset(::epoll_create(1));
	if (epfd_ == -1)
		throw std::system_error(errno, std::system_category(), "epoll_create");
}

template <class Handler>
void event_reactor::add_fd_watch(int fd, int events, Handler handler)
{
	epoll_event ee {.events = uint32_t(events), .data = {.fd = fd} };
	if (-1 == ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ee))
	{
		if (errno == EEXIST)
		{
			if (-1 == ::epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ee))
				throw std::system_error(errno, std::system_category(), "epoll_ctl");
		}
		else
			throw std::system_error(errno, std::system_category(), "epoll_ctl");
	}
	callbacks_[fd] = std::move(handler);
}

inline void event_reactor::delete_fd_watch(int fd)
{
	if (-1 == ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL))
		throw std::system_error(errno, std::system_category(), "epoll_ctl");
	callbacks_.erase(fd);
}

inline void event_reactor::run()
{
	constexpr int max_events = 10;
	constexpr int timeout_ms = 1000;
	epoll_event ees[max_events];
	while (1)
	{
		int nevents = ::epoll_wait(epfd_, ees, max_events, timeout_ms);
		if (nevents == -1)
		{
			if (errno == EINTR)
				continue;
			else
				throw std::system_error(errno, std::system_category(), "epoll_wait");
		}
		for (int i = 0; i < nevents; ++i)
		{
			assert(callbacks_.find(ees[i].data.fd) != callbacks_.end());
			callbacks_[ees[i].data.fd](ees[i].data.fd, ees[i].events);
		}
	}
}

#endif // _VLED_EVENT_REACTOR_H_
