#ifndef IO_HPP
# define IO_HPP

#include "Epoll.hpp"
#include <sys/epoll.h>

class IO{
private:
	IO();
	~IO();
public:
	static void	try_write(Epoll& epoll, struct epoll_event& event);
	static int	try_read(Epoll& epoll, struct epoll_event& event);

};

#endif // !IO_HPP
