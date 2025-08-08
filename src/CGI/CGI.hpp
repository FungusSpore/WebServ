#ifndef CGI_HPP
# define CGI_HPP

#include <sys/types.h>
#include <string>

class CGI{
private:
	CGI();
	~CGI();
public:
	static int exec(const char *cgi_path, std::string mime_type, char **envp, int epollfd, int clientfd);
};

#endif // !CGI_HPP
