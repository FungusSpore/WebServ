#include "../../includes/CGI.hpp"

int CGI::exec(const char *cgi_path, std::string mime_type, char **envp, Epoll& epoll, Socket *client){
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
		ev.data.ptr = epoll.makeClientSocket(pipefd[0], client);
		epoll_ctl(epoll.get_epollfd(), EPOLL_CTL_ADD, pipefd[0], &ev);
	}
	delete[] argv;
	return (0);
}

// int main(int argc, char **argv, char **envp){
// 	try {
// 		std::vector<std::string> port; 
// 		port.push_back("1234");
// 		Epoll epoll(port);
// 		char buffer[READ_BUFFER];
//
// 		for (;;){
// 			std::vector<Socket *> mysocks = epoll.get_conn_sock();
// 			for (size_t i = 0; i < mysocks.size(); i++){
// 				Socket *mysock = mysocks.at(i);
// 				while (true){
// 					int size = read(mysock->fd, buffer, READ_BUFFER);
// 					if (size == -1){
// 						if (errno == EAGAIN || errno == EWOULDBLOCK)
// 							break;
// 						throw  SystemFailure("Read Failed");
// 					}
// 					if (size == 0){
// 						epoll.closeSocket(*mysock);
// 						break ;
// 					}
// 					if (buffer[0] == 'z'){
// 						std::string cgi_path = "cgi-bin/hello_process.py";
// 						CGI::exec(cgi_path.c_str(), ".py", envp, epoll, mysock);
// 						break ;
// 					}
//
// 					else if (mysock->toSend)
// 						write(mysock->toSend->fd, buffer, size);
// 					else
// 						write(mysock->fd, buffer, size);
// 				}
// 			}
// 		}
//
// 	} catch (std::exception& e) {
// 		std::cerr << e.what() << std::endl;
//
// 	}
// }
