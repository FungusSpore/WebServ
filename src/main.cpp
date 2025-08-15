#include <iostream>
#include <vector>
#include "Epoll.hpp"
#include "Exceptions.hpp"
#include "MiniHttp.hpp"
#include "IO.hpp"

int main(int ac, char **av) {
	if (ac != 1) {
		std::cerr << "Usage: " << av[0] << " <path/to/server.conf>" << std::endl;
		return 1;
	}

	try {
		WebServer ProphetServer(av[1]);

		Epoll epoll(ProphetServer.getPorts(), ProphetServer);

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
						if (mysock->clientFd) {
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

	return 0;
}

