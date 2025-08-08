#ifndef UTILS_HPP
# define UTILS_HPP

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

class Utils{
private:
	Utils();
	~Utils();
public:
	static void setnonblocking(int socket);
	static std::vector<std::string> splitString(const std::string input, char delimiter);

};

#endif // !UTILS_HPP
