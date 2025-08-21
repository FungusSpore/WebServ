#include "../../includes/CGI.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int CGI::exec(const char *cgi_path, char **envp, Epoll& epoll, Socket& client){
	int sv[2];
	std::string interpreter;
	char **argv;
	struct epoll_event ev;

	argv = new char*[2];
	argv[0] = const_cast<char *>(cgi_path);
	argv[1] = NULL;

	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, sv) == -1)
		throw SystemFailure("cgi socketpair failed");

	pid_t pid = fork();
	switch (pid) {
	case -1:
		close(sv[0]);
		close(sv[1]);
		throw SystemFailure("CGI Fork failed");
	case 0:
		close(sv[0]);
		dup2(sv[1], STDOUT_FILENO);
		dup2(sv[1], STDIN_FILENO);
		close(sv[1]);
		execve(argv[0], argv, envp); // no need to check return value, it will exit if it fails - compile error 
		exit(1);
	default:
		close(sv[1]);
		ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
		ev.data.ptr = epoll.makeClientSocket(sv[0], client.fd);
		epoll_ctl(epoll.get_epollfd(), EPOLL_CTL_ADD, sv[0], &ev);
	}
	delete[] argv;
	return (sv[0]);
}

std::string extractHeader(std::string& read_buffer){
	std::string header;

	std::string::size_type s = read_buffer.find("\r\n\r\n");
	if (s == std::string::npos)
		return header;
	header = read_buffer.substr(0, s + 4);
	read_buffer.erase(0, s + 4);
	return header;
}

// read -> store in buffer -> check is it a complete request (Vicky)
// -> if not then store in buffer, and go next
// -> else then process and send back data (Vicky)
//		-> if can't send all put remainder in write_buffer go next
//		-> set EPOLLOUT
//
// write -> get data from write_buffer can write into socket
//				-> if can't send all go next
//				-> else remove EPOLLOUT

// int main(int argc, char **argv, char **envp){
// 	try {
// 		(void)argc;
// 		(void)argv;
// 		std::vector<std::string> port; 
// 		port.push_back("12346");
// 		// port.push_back("12345");
// 		Epoll epoll(port);
//
// 		for (;;){
// 			std::vector<struct epoll_event> myevents = epoll.get_conn_sock();
// 			for (size_t i = 0; i < myevents.size(); i++){
// 				Socket *mysock = static_cast<Socket *>(myevents.at(i).data.ptr);
// 				uint32_t events = myevents.at(i).events;
// 				if (events & EPOLLHUP){
// 					std::cout << "EPOLLHUP" << std::endl;
// 					epoll.closeSocket(*mysock);
// 					continue ;
// 				}
// 				if (events & EPOLLERR){
// 					std::cout << "EPOLLERR" << std::endl;
// 					epoll.closeSocket(*mysock);
// 					continue ;
// 				}
// 				if (events & EPOLLIN){
// 					std::cout << "EPOLLIN" << std::endl;
// 					// act like process
// 					if (IO::try_read(epoll, myevents.at(i)) != -1){
// 						// GET
// 						if (mysock->read_buffer.at(0) == 'G'){
// 							std::ifstream file("public/hello_form.html");
// 							if (!file.is_open()){
// 								std::cerr << "html fail to open" << std::endl;
// 								continue;
// 							}
// 							std::stringstream buffer;
// 							buffer << file.rdbuf();
//
// 							std::string body = buffer.str();
//
// 							// build proper HTTP/1.1 response
// 							std::string response =
// 									"HTTP/1.1 200 OK\r\n"
// 									"Content-Type: text/html\r\n"
// 									"Content-Length: " + std::to_string(body.size()) + "\r\n"
// 									"Connection: close\r\n" +
// 									"\r\n" + 
// 									body;
//
// 							mysock->write_buffer = response;
// 							IO::try_write(epoll, myevents.at(i));
// 							mysock->read_buffer.erase();
// 						}
// 						// POST
// 						else if (mysock->read_buffer.at(0) == 'P'){
// 							std::string cgi_path = "cgi-bin/hello_process.py";
// 							int write_end = CGI::exec(cgi_path.c_str(), envp, epoll, *mysock);
// 							write(write_end, mysock->read_buffer.c_str(), mysock->read_buffer.size());
// 							if (shutdown(write_end, 1) == -1)
// 								throw SystemFailure("shutdown failed");
// 							mysock->read_buffer.erase();
// 						}
//
// 						else {
// 							std::string header = CGI::extractHeader(mysock->read_buffer);
// 							if (header.size() == 0) // error
// 								continue;
//
// 							mysock->write_buffer = 
// 									"HTTP/1.1 200 OK\r\n"
// 									"Content-Length: " + std::to_string(mysock->read_buffer.size()) + "\r\n" +
// 									header +
// 									mysock->read_buffer;
//
// 							mysock->read_buffer.erase();
// 							std::cout << "fd " << mysock->fd << mysock->write_buffer << std::endl;
// 							IO::try_write(epoll, myevents.at(i));
// 						}
//
// 					}
// 				}
//
// 				if (events & EPOLLOUT){
// 					std::cout << "EPOLLOUT" << std::endl;
// 					IO::try_write(epoll, myevents.at(i));
// 				}
// 			}
// 		}
//
// 	} catch (std::exception& e) {
// 		std::cerr << e.what() << std::endl;
//
// 	}
// }
