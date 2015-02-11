#ifndef _VLED_MEMBUF_H_
#define _VLED_MEMBUF_H_

#include <streambuf>

/* std::streambuf wrapper around memory region. */
template <class CharT, class Traits = std::char_traits<CharT> >
class basic_membuf : public std::basic_streambuf<CharT, Traits>
{
public:
	typedef std::basic_streambuf<CharT, Traits> base_t;
	typedef typename base_t::char_type char_type;
	typedef typename base_t::traits_type traits_type;
	typedef typename base_t::int_type int_type;
	typedef typename base_t::pos_type pos_type;
	typedef typename base_t::off_type off_type;

	basic_membuf(char_type* s, std::streamsize n)
	{
		this->setg(s, s, s + n);
		this->setp(s, s + n);
	}

	basic_membuf(char_type* begin, char_type* end)
	{
		this->setg(begin, begin, end);
		this->setp(begin, end);
	}

	char_type* begin() const
	{
		return this->pbase();
	}

	char_type* end() const
	{
		return this->pptr();
	}

protected:
	virtual base_t* setbuf(char_type* s, std::streamsize n)
	{
		this->setg(s, s, s + n);
		this->setp(s, s + n);
		return this;
	}

	virtual int_type pbackfail(int_type c)
	{
		if (this->gptr() > this->eback())
		{
			this->gbump(-1);
			*this->gptr() = traits_type::to_char_type(c);
			return c;
		}
		else
			return traits_type::eof();
	}
};

typedef basic_membuf<char> membuf;
typedef basic_membuf<wchar_t> wmembuf;

#endif // _VLED_MEMBUF_H_
