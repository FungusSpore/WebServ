#include <iostream>
#include <vector>
#include <signal.h>
#include "Epoll.hpp"
#include "Exceptions.hpp"
#include "MiniHttp.hpp"
#include "IO.hpp"

volatile int g_signal = 0;

void signalHandler(int sig) {
	if (sig == SIGINT) {
		std::cout << "\nReceived SIGINT (Ctrl+C). Shutting down..." << std::endl;
		g_signal = 1;
	}
}

void initSignalHandler() {
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		std::cerr << "Error setting up signal handler" << std::endl;
		exit(1);
	}
}

int main(int ac, char **av) {
	if (ac != 2) {
		std::cerr << "Usage: " << av[0] << " <path/to/server.conf>" << std::endl;
		return 1;
	}


	initSignalHandler();
	try {
		WebServer ProphetServer(av[1]);

		Epoll epoll(ProphetServer.getPorts(), ProphetServer);

		std::cout << "WebServer started. Press Ctrl+C to shut down." << std::endl;

		while (!g_signal){
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
						if (mysock->clientFd != -1) {
							if (mysock->validateCGI())
								IO::try_write(epoll, myevents.at(i));
							// std::string cgi_path = "cgi-bin/hello_process.py";
							// int write_end = CGI::exec(cgi_path.c_str(), ".py", envp, epoll, *mysock);
							// // dont' need
							// if (shutdown(write_end, 1) == -1)
							// 	throw SystemFailure("shutdown failed");
							// mysock->read_buffer.erase();
						}
						else {
							if (mysock->runHttp()) 
								IO::try_write(epoll, myevents.at(i));

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

		// Explicit cleanup before exit
		std::cout << "Server shutting down..." << std::endl;
		std::cout << "All sockets closed. Ports should be available immediately." << std::endl;

	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;

	}

	return 0;
}

