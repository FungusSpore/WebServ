#include "../../includes/CGI.hpp"

int CGI::exec(const char *cgi_path, std::string mime_type, char **envp, Epoll& epoll, Socket& client){
	int sv[2];
	std::string interpreter;
	char **argv;
	struct epoll_event ev;

	if (mime_type == ".py")
		interpreter = "/usr/bin/python3";
	argv = new char*[3];
	argv[0] = const_cast<char *>(interpreter.c_str());
	argv[1] = const_cast<char *>(cgi_path);
	argv[2] = NULL;

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
		if (execve(interpreter.c_str(), argv, envp) == -1)
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
// 		std::vector<std::string> port; 
// 		port.push_back("12345");
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
//
// 				if (events & EPOLLIN){
// 					std::cout << "EPOLLIN" << std::endl;
// 					// act like process
// 					if (IO::try_read(epoll, myevents.at(i)) != -1){
// 						if (mysock->read_buffer.at(0) == 'z'){
// 							std::string cgi_path = "cgi-bin/hello_process.py";
// 							int write_end = CGI::exec(cgi_path.c_str(), ".py", envp, epoll, *mysock);
// 							// dont' need
// 							if (shutdown(write_end, 1) == -1)
// 								throw SystemFailure("shutdown failed");
// 							mysock->read_buffer.erase();
// 						}
// 						else {
// 							mysock->write_buffer = mysock->read_buffer;
// 							mysock->read_buffer.erase();
// 							std::cout << "fd " << mysock->fd << mysock->write_buffer << std::endl;
// 							IO::try_write(epoll, myevents.at(i));
// 						}
// 					}
// 				}
//
// 				if (events & EPOLLOUT){
// 					std::cout << "EPOLLOUT" << std::endl;
// 					IO::try_write(epoll, myevents.at(i));
// 				}
//
//
// 			}
// 		}
//
// 	} catch (std::exception& e) {
// 		std::cerr << e.what() << std::endl;
//
// 	}
// }
