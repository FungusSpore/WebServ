#ifndef CGI_HPP
# define CGI_HPP

#include "Socket.hpp"
#include "Epoll.hpp"
#include "Exceptions.hpp"
#include "Utils.hpp"

#include <sys/types.h>
#include <sys/epoll.h>
#include <string>
#include <cerrno>
#include <unistd.h>

class CGI{
private:
	CGI();
	~CGI();
public:
	static int exec(const char *cgi_path, std::string mime_type, char **envp, Epoll& epoll, Socket* client);
};

#endif // !CGI_HPP
