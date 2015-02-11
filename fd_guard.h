#ifndef _VLED_FD_GUARD_H_
#define _VLED_FD_GUARD_H_

#include <unistd.h>

/* RAII wrapper for file descriptors. */
class fd_guard
{
public:
	explicit fd_guard(int fd = -1)
	 : fd_(fd)
	{}
	fd_guard(const fd_guard&) = delete;
	fd_guard(fd_guard&& o) : fd_(o.fd_)
	{
		o.fd_ = -1;
	}
	fd_guard& operator=(const fd_guard&) = delete;
	fd_guard& operator=(fd_guard&& o)
	{
		fd_ = o.fd_;
		o.fd_ = -1;
		return *this;
	}

	operator int() const
	{
		return fd_;
	}

	void reset(int fd = -1)
	{
		if (fd_ != -1)
			::close(fd_);
		fd_ = fd;
	}

	~fd_guard()
	{
		if (fd_ != -1)
			::close(fd_);
	}

private:
	int fd_;
};

#endif // _VLED_FD_GUARD_H_
