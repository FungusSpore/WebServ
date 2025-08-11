#include <iostream>
#include <vector>
#include "Epoll.hpp"
#include "Exceptions.hpp"
#include "MiniHttp.hpp"

int main() {
	WebServer ProphetServer;


	std::vector<std::string> port_list;
	port_list.push_back("8080");
	port_list.push_back("8081");
	port_list.push_back("8082");

	Epoll ProphetSockets(port_list);
	try {
		while (true) {
			std::vector<Socket *> sockets = ProphetSockets.get_conn_sock();
			for (std::vector<Socket *>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
				Socket *sock = *it;

				MiniHttp ProphetHttp(sock, ProphetServer);
				ProphetHttp.run();
			}
		}
	}
	catch (const std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}


	return 0;
}

