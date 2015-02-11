#include "virtual_led.h"
#include "event_reactor.h"

#include <sys/timerfd.h>

#include <system_error>

virtual_led::virtual_led(event_reactor* er)
 : on_(true)
 , color_(color::red)
 , rate_(0)
 , blink_on_(true)
 , er_(er)
{
 	tfd_.reset(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK));
	if (tfd_ == -1)
		throw std::system_error(errno, std::system_category(), "timerfd_create");
		
	er_->add_fd_watch(tfd_, event_reactor::e_read, std::bind(&virtual_led::handle_timer, this));
	draw();
}

virtual_led::~virtual_led()
{
	er_->delete_fd_watch(tfd_);
	on_ = false;
	draw();
}

void virtual_led::draw() const
{
	if (on_ && blink_on_)
	{
		// Not using color output because the correct way of doing that is getting
		// escape codes from terminfo DB using ncurses and I'm not allowed to do that.
		switch (color_)
		{
		case color::red:
			printf("\rRed  ");
			break;
		case color::green:
			printf("\rGreen");
			break;
		case color::blue:
			printf("\rBlue ");
			break;
		default:
			printf("\r     ");
			break;
		}
	}
	else
		printf("\r     ");
	fflush(stdout);
}

void virtual_led::handle_timer()
{
	// Have to read timer expiration counter to clear it
	uint64_t unused;
	while (-1 != ::read(tfd_, &unused, sizeof(unused)))
		;
	if (errno != EAGAIN)
		throw std::system_error(errno, std::system_category(), "read");

	if (rate_ > 0)
		blink_on_ = !blink_on_;
	draw();
}

void virtual_led::setup_timer()
{
	struct itimerspec its;
	if (on_ && rate_ > 0)
	{
		its.it_interval.tv_sec = time_t(1/rate_);
		its.it_interval.tv_nsec = long(1000000000L/rate_ % 1000000000L);
	}
	else
	{
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
	}
	its.it_value = its.it_interval;
	if (-1 == ::timerfd_settime(tfd_, 0, &its, NULL))
		throw std::system_error(errno, std::system_category(), "timerfd_settime");
}

void virtual_led::on()
{
	if (!on_)
	{
		on_ = true;
		draw();
		setup_timer();
	}
}

void virtual_led::off()
{
	if (on_)
	{
		on_ = false;
		draw();
		setup_timer();
	}
}

void virtual_led::set_color(color c)
{
	if (color_ != c)
	{
		color_ = c;
		draw();
	}
}

void virtual_led::set_rate(unsigned long r)
{
	rate_ = r;
	setup_timer();
	if (rate_ == 0)
	{
		blink_on_ = true;
		draw();
	}
}

bool virtual_led::is_on() const
{
	return on_;
}

virtual_led::color virtual_led::get_color() const
{
	return color_;
}

unsigned long virtual_led::get_rate() const
{
	return rate_;
}
