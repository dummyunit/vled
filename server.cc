#include "server.h"
#include "event_reactor.h"
#include "membuf.h"

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <system_error>

const size_t in_buffer_size = 64;
const size_t out_buffer_size = 64;

void create_fifo(const char* path)
{
	if (-1 == ::mkfifo(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
	{
		if (errno == EEXIST)
		{
			struct stat st;
			if (-1 == ::stat(path, &st))
				throw std::system_error(errno, std::system_category(), "stat");
			if (!(st.st_mode & S_IFIFO))
				throw std::runtime_error(std::string({"File exists and it is not a fifo: ", path}));
		}
		else
			throw std::system_error(errno, std::system_category(), "mkfifo");
	}
}

server::server(event_reactor* er, const char* in_fifo_path, const char* out_fifo_path)
  : er_(er)
{
	using namespace std::placeholders;

	create_fifo(in_fifo_path);
	create_fifo(out_fifo_path);

	// O_RDWR instead of O_RDONLY to avoid getting endless stream of EPOLLHUP
	// after first client closes the pipe.
	fdin_.reset(::open(in_fifo_path, O_RDWR | O_NONBLOCK));
	if (fdin_ == -1)
		throw std::system_error(errno, std::system_category(), "open");
	// O_RDWR instead of O_WRONLY because attempt to open with O_WRONLY and O_NONBLOCK
	// results in ENXIO. Also this shields from SIGPIPE because there is always at least 1 reader.
	fdout_.reset(::open(out_fifo_path, O_RDWR | O_NONBLOCK));
	if (fdout_ == -1)
		throw std::system_error(errno, std::system_category(), "open");
	
	parser_.set_request_handler(std::bind(&server::handle_request, this, _1, _2));
	parser_.set_error_handler(std::bind(&server::handle_parse_error, this));
}

server::~server()
{
	er_->delete_fd_watch(fdin_);
}

void server::start()
{
	er_->add_fd_watch(fdin_, event_reactor::e_read, std::bind(&server::handle_read, this));
}

void server::handle_read()
{
	char buf[in_buffer_size];
	ssize_t nread;
	if (-1 == (nread = ::read(fdin_, buf, in_buffer_size)))
	{
		if (errno == EAGAIN)
			return;
		else
			throw std::system_error(errno, std::system_category(), "read");
	}
	const char* pbegin = buf;
	const char* pend = buf + nread;
	parser_.parse(pbegin, pend);
	if (pbegin != pend)
		throw std::runtime_error("parsing client request failed");
}

void server::handle_request(const std::string& command, const std::string& argument)
{
	response r{false};

	auto cmd_it = command_handlers_.find(command);
	if (cmd_it != command_handlers_.end())
	{
		// Ignore and log error caused by command handlers because they do not threaten server
		// functionality (aka. handling pipe I/O).
		try { r = cmd_it->second(argument); }
		catch (const std::exception& e)
		{
			std::cerr << "caught exception while processing client request: " << e.what() << '\n';
		}
	}

	write_response(r);
}

bool server::handle_parse_error()
{
	write_response(response {false});
	return false;
}

void server::write_response(const response& r)
{
	char buf[out_buffer_size];
	membuf mbuf(std::begin(buf), std::end(buf));
	std::ostream os(&mbuf);
	if (r.success)
		os << "OK";
	else
		os << "FAILURE";
	if (!r.result.empty())
		os << ' ' << r.result;
	os << '\n';

	size_t towrite = std::distance(mbuf.begin(), mbuf.end());
	size_t nwrite_total = 0;
	while (nwrite_total < towrite)
	{
		ssize_t nwrite = ::write(fdout_, mbuf.begin() + nwrite_total, towrite - nwrite_total);
		if (nwrite == -1)
			throw std::system_error(errno, std::system_category(), "write");
		else
		{
			assert(nwrite > 0);
			nwrite_total += nwrite;
		}
	}
}
