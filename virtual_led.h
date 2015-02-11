#ifndef _VLED_VIRTUAL_LED_H_
#define _VLED_VIRTUAL_LED_H_

#include "event_reactor_fwd.h"
#include "fd_guard.h"

/* LED state and visualization. */
class virtual_led
{
public:
	enum class color
	{
		red,
		green,
		blue,
	};

	virtual_led(event_reactor* er);
	~virtual_led();

	void on();
	void off();
	void set_color(color c);
	void set_rate(unsigned long r);

	bool is_on() const;
	color get_color() const;
	unsigned long get_rate() const;

private:
	void draw() const;
	void handle_timer();
	void setup_timer();

private:
	bool on_;
	color color_;
	unsigned long rate_;
	bool blink_on_;

	fd_guard tfd_;
	event_reactor* er_;
};

#endif // _VLED_VIRTUAL_LED_H_
