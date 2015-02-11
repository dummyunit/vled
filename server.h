#ifndef _VLED_SERVER_H_
#define _VLED_SERVER_H_

#include "event_reactor_fwd.h"
#include "protocol_parser.h"
#include "fd_guard.h"

#include <functional>
#include <map>
#include <string>
#include <utility>

/* Generic server that works with simple text protocol. */
class server
{
public:
	struct response
	{
		bool success;
		std::string result;
	};

	server(event_reactor* er, const char* in_fifo_path, const char* out_fifo_path);
	~server();

	template <class Handler>
	void add_command(std::string&& name, Handler handler);
	template <class Handler>
	void add_command(const std::string& name, Handler handler);

	void remove_command(const std::string& name);
	void remove_command(const char* name);

	void start();

private:
	void handle_read();
	void handle_request(const std::string& command, const std::string& argument);
	bool handle_parse_error();
	void write_response(const response& r);

private:
	fd_guard fdin_;
	fd_guard fdout_;
	protocol_parser parser_;

	std::map<std::string, std::function<response (std::string)>> command_handlers_;

	event_reactor* er_;
};

template <class Handler>
void server::add_command(std::string&& name, Handler handler)
{
	command_handlers_.emplace(std::move(name), std::move(handler));
}

template <class Handler>
void server::add_command(const std::string& name, Handler handler)
{
	command_handlers_.emplace(name, std::move(handler));
}

inline void server::remove_command(const std::string& name)
{
	command_handlers_.erase(name);
}

inline void server::remove_command(const char* name)
{
	auto it = command_handlers_.find(name);
	if (it != command_handlers_.end())
		command_handlers_.erase(it);
}	

#endif // _VLED_SERVER_H_
