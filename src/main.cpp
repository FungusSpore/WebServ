#include <iostream>
#include <vector>
#include <signal.h>
#include "../includes/Epoll.hpp"
#include "../includes/Exceptions.hpp"
#include "../includes/MiniHttp.hpp"
#include "../includes/IO.hpp"

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
		exit(1); } }

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
				if (events & EPOLLERR) {
						std::cout << "EPOLLERR on socket " << mysock->fd << ": \n";
						
						// Get the specific socket error
						int socket_error = 0;
						socklen_t len = sizeof(socket_error);
						int sockfd = mysock->fd;
						
						if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &socket_error, &len) == 0 && socket_error != 0) {
								switch (socket_error) {
										case ECONNRESET:
												std::cout << "Connection reset by peer";
												break;
										case EPIPE:
												std::cout << "Broken pipe (remote end closed)";
												break;
										case ENOBUFS:
												std::cout << "No buffer space available";
												break;
										case ENOMEM:
												std::cout << "Out of memory";
												break;
										case ENETDOWN:
												std::cout << "Network is down";
												break;
										case ENETUNREACH:
												std::cout << "Network is unreachable";
												break;
										case EHOSTUNREACH:
												std::cout << "Host is unreachable";
												break;
										case ETIMEDOUT:
												std::cout << "Connection timed out";
												break;
										case ECONNREFUSED:
												std::cout << "Connection refused";
												break;
										case ENOTCONN:
												std::cout << "Socket is not connected";
												break;
										default:
												std::cout << strerror(socket_error) << " (errno: " << socket_error << ")";
												break;
								}
						} else {
								// Fallback: use current errno if getsockopt fails
								std::cout << strerror(errno) << " (errno: " << errno << ")";
						}
						
						std::cout << std::endl;
						epoll.closeSocket(*mysock);
						continue;
				}

				if (events & EPOLLIN){
					std::cout << "EPOLLIN on socket " << mysock->fd << ": \n";
					// act like process
					if (IO::try_read(epoll, myevents.at(i)) != -1){
						// need to detect if this is a cgi response to continue
						if (mysock->toSend != NULL) {
							if (mysock->validateCGI())
								IO::try_write(epoll, myevents.at(i));
						}
						else {
							if (mysock->runHttp()) {
								IO::try_write(epoll, myevents.at(i));
							}
							else if (mysock->isCgi && !mysock->executeCGI(epoll)) {
								IO::try_write(epoll, myevents.at(i));
							}
						}
					}
				}

				if (events & EPOLLOUT){
					std::cout << "EPOLLOUT on socket " << mysock->fd << ": \n";
					IO::try_write(epoll, myevents.at(i));
				}
				if (events & EPOLLHUP){
					std::cout << "EPOLLHUP on socket " << mysock->fd << ": \n";
					epoll.closeSocket(*mysock);
					continue ;
				}
			}
		}

		// Explicit cleanup before exit
		std::cout << "Server shutting down..." << std::endl;
		std::cout << "All sockets closed. Ports should be available immediately." << std::endl;

	} catch (std::exception& e) {
		std::cerr << "Error" << e.what() << std::endl;

	}

	return 0;
}

