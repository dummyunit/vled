#include "event_reactor.h"
#include "virtual_led.h"
#include "server.h"

#include <algorithm>
#include <cctype>
#include <string>

#define IN_FIFO_PATH "/tmp/vledd.fifo_in"
#define OUT_FIFO_PATH "/tmp/vledd.fifo_out"

int main()
{
	event_reactor er;
	virtual_led led(&er);
	server srv(&er, IN_FIFO_PATH, OUT_FIFO_PATH);

	srv.add_command("set-led-state", [&led](const std::string& arg) -> server::response
	{
		if (arg == "on")
		{
			led.on();
			return server::response {true};
		}
		else if (arg == "off")
		{
			led.off();
			return server::response {true};
		}
		else
			return server::response {false};
	});
	srv.add_command("get-led-state", [&led](const std::string&) -> server::response
	{
		return server::response {true, led.is_on() ? "on" : "off"};
	});
	srv.add_command("set-led-color", [&led](const std::string& arg) -> server::response
	{
		if (arg == "red")
		{
			led.set_color(virtual_led::color::red);
			return server::response {true};
		}
		else if (arg == "green")
		{
			led.set_color(virtual_led::color::green);
			return server::response {true};
		}
		else if (arg == "blue")
		{
			led.set_color(virtual_led::color::blue);
			return server::response {true};
		}
		else
			return server::response {false};
	});
	srv.add_command("get-led-color", [&led](const std::string&) -> server::response
	{
		switch (led.get_color())
		{
		case virtual_led::color::red:
			return server::response {true, "red"};
		case virtual_led::color::green:
			return server::response {true, "green"};
		case virtual_led::color::blue:
			return server::response {true, "blue"};
		default:
			return server::response {false};
		}
	});
	srv.add_command("set-led-rate", [&led](const std::string& arg) -> server::response
	{
		unsigned long rate = std::stoul(arg);
		if (rate > 5)
			return server::response {false};
		led.set_rate(rate);
		return server::response {true};
	});
	srv.add_command("get-led-rate", [&led](const std::string&) -> server::response
	{
		return server::response {true, std::to_string(led.get_rate())};
	});

	srv.start();
	er.run();
	return 0;
}
