#ifndef CGI_HPP
# define CGI_HPP

#include "Socket.hpp"
#include "Epoll.hpp"
#include "Exceptions.hpp"
#include "Utils.hpp"
#include "IO.hpp"

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <string>
#include <cerrno>
#include <unistd.h>

class CGI{
private:
	CGI();
	~CGI();
public:
	static int exec(const char *cgi_path, char **envp, Epoll& epoll, Socket& client);
	static std::string extractHeader(std::string& read_buffer);
};

#endif // !CGI_HPP
