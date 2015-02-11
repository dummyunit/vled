#ifndef _VLED_PROTOCOL_PARSER_H_
#define _VLED_PROTOCOL_PARSER_H_

#include <cctype>
#include <functional>
#include <string>
#include <utility>

/* Push parser implemented using finite state machine. */
class protocol_parser
{
public:
	enum class error
	{
		missing_command,
		extra_argument,
	};

	protocol_parser();

	template <class RequestHandler>
	void set_request_handler(RequestHandler handler);

	template <class ErrorHandler>
	void set_error_handler(ErrorHandler handler);

	template <class InputIterator>
	void parse(InputIterator& begin, InputIterator end);

	void reset();

private:
	void process_request();

private:
	enum class state
	{
		initial,
		command,
		command_end,
		argument,
		argument_end,
		error,
	} state_;

	std::string cur_command_;
	std::string cur_argument_;

	std::function<void (const std::string&, const std::string&)> request_handler_;
	std::function<bool (error)> error_handler_;
};

inline protocol_parser::protocol_parser()
{
	reset();
}

template <class RequestHandler>
void protocol_parser::set_request_handler(RequestHandler handler)
{
	request_handler_ = std::move(handler);
}

template <class ErrorHandler>
void protocol_parser::set_error_handler(ErrorHandler handler)
{
	error_handler_ = std::move(handler);
}

inline void protocol_parser::reset()
{
	cur_command_.clear();
	cur_argument_.clear();
	state_ = state::initial;
}

inline void protocol_parser::process_request()
{
	if (request_handler_)
		request_handler_(cur_command_, cur_argument_);
}

template <class InputIterator>
void protocol_parser::parse(InputIterator& begin, InputIterator end)
{
	for (; begin != end; ++begin)
	{
		switch (state_)
		{
		case state::initial:
			if (*begin == '\n')
			{
				if (error_handler_)
					if (error_handler_(error::missing_command))
						return;
				state_ = state::error;
			}
			else if (std::isblank(*begin))
				state_ = state::initial;
			else
			{
				cur_command_.push_back(*begin);
				state_ = state::command;
			}
			break;
		case state::command:
			if (*begin == '\n')
			{
				process_request();
				reset();
			}
			else if (std::isblank(*begin))
				state_ = state::command_end;
			else
			{
				cur_command_.push_back(*begin);
				state_ = state::command;
			}
			break;
		case state::command_end:
			if (*begin == '\n')
			{
				process_request();
				reset();
			}
			else if (std::isblank(*begin))
				state_ = state::command_end;
			else
			{
				cur_argument_.push_back(*begin);
				state_ = state::argument;
			}
			break;
		case state::argument:
			if (*begin == '\n')
			{
				process_request();
				reset();
			}
			else if (std::isblank(*begin))
				state_ = state::argument_end;
			else
			{
				cur_argument_.push_back(*begin);
				state_ = state::argument;
			}
			break;
		case state::argument_end:
			if (*begin == '\n')
			{
				process_request();
				reset();
			}
			else if (std::isblank(*begin))
				state_ = state::argument_end;
			else
			{
				if (error_handler_)
					if (error_handler_(error::extra_argument))
						return;
				state_ = state::error;
			}
			break;
		case state::error:
			if (*begin == '\n')
				reset();
			else
				state_ = state::error;
			break;
		}
	}
}

#endif // _VLED_PROTOCOL_PARSER_H_
