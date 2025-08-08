#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

#include <exception>
#include <string>

class SystemFailure : public std::exception{
private:
	std::string msg;
public:
	SystemFailure();
	SystemFailure(std::string msg);
	const char* what() const throw();
	~SystemFailure() throw();
};

#endif
