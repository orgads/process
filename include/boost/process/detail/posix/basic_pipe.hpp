// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_POSIX_PIPE_HPP
#define BOOST_PROCESS_POSIX_PIPE_HPP


#include <boost/filesystem.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/process/detail/posix/compare_handles.hpp>
#include <system_error>
#include <array>
#include <unistd.h>
#include <memory>

namespace boost { namespace process { namespace detail { namespace posix {


template<class CharT, class Traits = std::char_traits<CharT>>
class basic_pipe
{
    int _source = -1;
    int _sink   = -1;
public:
    basic_pipe(int source, int sink) : _source(source), _sink(sink) {}
    basic_pipe(int source, int sink, const std::string & name) : _source(source), _sink(sink) {}
    typedef CharT                      char_type  ;
    typedef          Traits            traits_type;
    typedef typename Traits::int_type  int_type   ;
    typedef typename Traits::pos_type  pos_type   ;
    typedef typename Traits::off_type  off_type   ;
    typedef 		 int			   native_handle;

    basic_pipe()
    {
		int fds[2];
		if (::pipe(fds) == -1)
			boost::process::detail::throw_last_error("pipe(2) failed");

		_source = fds[0];
		_sink   = fds[1];
    }
    basic_pipe(const basic_pipe& rhs);
    basic_pipe(const std::string& name);
    basic_pipe(basic_pipe&& lhs)  : _source(lhs._source), _sink(lhs._sink)
    {
        lhs._source = -1;
        lhs._sink   = -1;
    }
    basic_pipe& operator=(const basic_pipe& );
    basic_pipe& operator=(basic_pipe&& lhs)
    {
        _source = lhs._source;
        _sink   = lhs._sink ;

        lhs._source = -1;
        lhs._sink   = -1;

        return *this;
    }
    ~basic_pipe()
    {
        if (_sink   != -1)
            ::close(_sink);
        if (_source != -1)
            ::close(_source);
    }
    native_handle native_source() const {return _source;}
    native_handle native_sink  () const {return _sink;}


    int_type write(const char_type * data, int_type count)
    {
    	auto write_len = ::write(_sink, data, count * sizeof(char_type));
    	if (write_len == -1)
    		::boost::process::detail::throw_last_error();

        return write_len;
    }
    int_type read(char_type * data, int_type count)
    {
    	auto read_len = ::read(_source, data, count * sizeof(char_type));
    	if (read_len == -1)
    		::boost::process::detail::throw_last_error();

        return read_len;
    }

    bool is_open()
    {
        return (_source != -1) ||
               (_sink   != -1);
    }

    void close()
    {
        ::close(_source);
        ::close(_sink);
        _source = -1;
        _sink   = -1;
    }
};

template<class CharT, class Traits>
basic_pipe<CharT, Traits>::basic_pipe(const basic_pipe & rhs)
{
   	if (rhs._source != -1)
   	{
   		_source = ::dup(rhs._source);
   		if (_source == -1)
   			::boost::process::detail::throw_last_error("dup() failed");
   	}
	if (rhs._sink != -1)
	{
		_sink = ::dup(rhs._sink);
		if (_sink == -1)
			::boost::process::detail::throw_last_error("dup() failed");

	}
}

template<class CharT, class Traits>
basic_pipe<CharT, Traits> &basic_pipe<CharT, Traits>::operator=(const basic_pipe & rhs)
{
   	if (rhs._source != -1)
   	{
   		_source = ::dup(rhs._source);
   		if (_source == -1)
   			::boost::process::detail::throw_last_error("dup() failed");
   	}
	if (rhs._sink != -1)
	{
		_sink = ::dup(rhs._sink);
		if (_sink == -1)
			::boost::process::detail::throw_last_error("dup() failed");

	}
	return *this;
}


template<class CharT, class Traits>
basic_pipe<CharT, Traits>::basic_pipe(const std::string & name)
{
    auto fifo = mkfifo(name.c_str(), 0666 );
            
    if (fifo != 0) 
        boost::process::detail::throw_last_error("mkfifo() failed");

    
    int  read_fd = open(name.c_str(), O_RDWR);
        
    if (read_fd == -1)
        boost::process::detail::throw_last_error();
    
    int write_fd = dup(read_fd);
    
    if (write_fd == -1)
        boost::process::detail::throw_last_error();

    _sink = write_fd;
    _source = read_fd;
    ::unlink(name.c_str());
}


}}}}

#endif
