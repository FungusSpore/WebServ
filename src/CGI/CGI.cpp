#include "CGI.hpp"
#include "../../includes/Epoll.hpp"
#include "../../includes/Exceptions.hpp"
#include <cerrno>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include "../../includes/Utils.hpp"

int CGI::exec(const char *cgi_path, std::string mime_type, char **envp, int epollfd, int clientfd){
	int pipefd[2];
	std::string interpreter;
	char **argv;
	struct epoll_event ev;

	if (mime_type == ".py")
		interpreter = "/usr/bin/python3";
	argv = new char*[3];
	argv[0] = const_cast<char *>(interpreter.c_str());
	argv[1] = const_cast<char *>(cgi_path);
	argv[2] = NULL;

	if (pipe(pipefd) == -1)
		throw SystemFailure("CGI Pipe Failed");
	Utils::setnonblocking(pipefd[0]);

	pid_t pid = fork();
	switch (pid) {
	case -1:
		close(pipefd[0]);
		close(pipefd[1]);
		throw SystemFailure("CGI Fork failed");
	case 0:
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		close(pipefd[0]);
		if (execve(interpreter.c_str(), argv, envp) == -1)
			throw  SystemFailure("CGI execve failed");
	default:
		close(pipefd[1]);
		ev.events = EPOLLIN | EPOLLET;
		ev.data.ptr = new Socket(pipefd[0], clientfd);
		epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &ev);
	}
	return (0);
}

#include "../../includes/Epoll.hpp"
int main(int argc, char **argv, char **envp){
	try {
		std::vector<std::string> port; 
		port.push_back("1234");
		Epoll epoll(port);
		// std::vector<Socket *> sock_list = epoll.get_conn_sock();
		// sock_list = epoll.get_conn_sock();
		// std::string cgi_path = "cgi-bin/hello_process.py";
		// CGI::exec(cgi_path.c_str(), ".py", envp, epoll.get_epollfd(), sock_list.at(0)->fd);
		//
		char buffer[2048];
		// sock_list = epoll.get_conn_sock();
		// int size = read(sock_list.at(0)->fd, buffer, 2048);
		// write(sock_list.at(0)->forward_fd, buffer, size);

		bool run_once = false;
		for (;;){
			std::vector<Socket *> mysocks = epoll.get_conn_sock();
			std::string cgi_path = "cgi-bin/hello_process.py";
			CGI::exec(cgi_path.c_str(), ".py", envp, epoll.get_epollfd(), mysocks.at(0)->fd);
			for (size_t i = 0; i < mysocks.size(); i++){
				Socket *mysock = mysocks[i];
				while (true){
					int size = read(mysock->fd, buffer, READ_BUFFER);
					if (size == -1){
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						throw  SystemFailure("Read Failed");
					}
					if (mysock->forward_fd != -1)
						write(mysock->forward_fd, buffer, size);
					else
						write(mysock->fd, buffer, size);
				}
				close(mysock->fd);
				delete mysock;
			}
		}

	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	
	}
}
