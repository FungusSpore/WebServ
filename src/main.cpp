#include <iostream>
#include <vector>
#include "Epoll.hpp"
#include "Exceptions.hpp"
#include "MiniHttp.hpp"
#include "IO.hpp"

int main() {
	WebServer ProphetServer;


	try {
		std::vector<std::string> port; 
		port.push_back("80");
		port.push_back("8080");

		Epoll epoll(port, ProphetServer);

		for (;;){
			std::vector<struct epoll_event> myevents = epoll.get_conn_sock();
			for (size_t i = 0; i < myevents.size(); i++){
				Socket *mysock = static_cast<Socket *>(myevents.at(i).data.ptr);
				uint32_t events = myevents.at(i).events;
				if (events & EPOLLHUP){
					std::cout << "EPOLLHUP" << std::endl;
					epoll.closeSocket(*mysock);
					continue ;
				}
				if (events & EPOLLERR){
					std::cout << "EPOLLERR" << std::endl;
					epoll.closeSocket(*mysock);
					continue ;
				}

				if (events & EPOLLIN){
					std::cout << "EPOLLIN" << std::endl;
					// act like process
					if (IO::try_read(epoll, myevents.at(i)) != -1){
						// need to detect if this is a cgi response to continue
						if (mysock->cgiReturned) {
							// std::string cgi_path = "cgi-bin/hello_process.py";
							// int write_end = CGI::exec(cgi_path.c_str(), ".py", envp, epoll, *mysock);
							// // dont' need
							// if (shutdown(write_end, 1) == -1)
							// 	throw SystemFailure("shutdown failed");
							// mysock->read_buffer.erase();
						}
						else {
							mysock->runHttp();
							// mysock->write_buffer = mysock->read_buffer;
							// mysock->read_buffer.erase();
							// std::cout << "fd " << mysock->fd << mysock->write_buffer << std::endl;
							// IO::try_write(epoll, myevents.at(i));
						}
					}
				}

				if (events & EPOLLOUT){
					std::cout << "EPOLLOUT" << std::endl;
					IO::try_write(epoll, myevents.at(i));
				}


			}
		}

	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;

	}
	// std::vector<std::string> port_list;
	// port_list.push_back("8080");
	// port_list.push_back("8081");
	// port_list.push_back("8082");
	//
	// Epoll ProphetSockets(port_list);
	// try {
	// 	while (true) {
	// 		std::vector<Socket *> sockets = ProphetSockets.get_conn_sock();
	// 		for (std::vector<Socket *>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
	// 			Socket *sock = *it;
	//
	// 			MiniHttp ProphetHttp(sock, ProphetServer);
	// 			ProphetHttp.run();
	// 		}
	// 	}
	// }
	// catch (const std::exception &e) {
	// 	std::cerr << "Exception: " << e.what() << std::endl;
	// }
	//

	return 0;
}

