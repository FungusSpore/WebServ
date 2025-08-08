#include "../../includes/Exceptions.hpp"

SystemFailure::SystemFailure(){}

SystemFailure::SystemFailure(std::string msg):msg(msg){}

const char* SystemFailure::what() const throw(){
	return (this->msg.c_str());
}
	
SystemFailure::~SystemFailure() throw(){}
