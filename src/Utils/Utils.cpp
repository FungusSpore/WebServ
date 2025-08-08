#include "../../includes/Utils.hpp"
#include "../../includes/Exceptions.hpp"


void Utils::setnonblocking(int socket){
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1) {
		close(socket);
		throw SystemFailure("Failed to get flags");
	}

	flags |= O_NONBLOCK;
	if (fcntl(socket, F_SETFL, flags) == -1){
		close(socket);
		throw SystemFailure("Failed to set flags");
	}
}

std::vector<std::string> Utils::splitString(const std::string input, char delimiter){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(input);

	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);
	
	return (tokens);
}

// int main(){
// 	std::vector<std::string> tokens = Utils::splitString("Hi:Bye", ':');
// 	for (size_t i = 0; i < tokens.size(); i++)
// 		std::cout << tokens.at(i) << std::endl;
// }
