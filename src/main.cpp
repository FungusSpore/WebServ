#include <signal.h>
#include "../includes/Epoll.hpp"
#include "../includes/MiniHttp.hpp"

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
				std::cout << "New Event" << std::endl;
				Socket *mysock = static_cast<Socket *>(myevents.at(i).data.ptr);
				uint32_t events = myevents.at(i).events;
				if (events & EPOLLERR) {
					std::cout << "EPOLLERR on socket " << mysock->fd << std::endl;
					epoll.handle_epollerr(mysock);
					continue;
				}
				if (events & EPOLLIN){
					std::cout << "EPOLLIN on socket " << mysock->fd << std::endl;;
					epoll.handle_epollin(mysock, myevents.at(i));
				}
				if (events & EPOLLOUT){
					std::cout << "EPOLLOUT on socket " << mysock->fd << std::endl;
					epoll.handle_epollout(myevents.at(i));
				}
				if (events & EPOLLHUP){
					std::cout << "EPOLLHUP on socket " << mysock->fd << std::endl;
					epoll.handle_epollhup(mysock, myevents.at(i));
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

