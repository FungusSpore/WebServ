#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

void createConnection(int id, const char* host, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        return;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        return;
    }
    
    std::cout << "Connection " << id << " established" << std::endl;
    
    // Keep connection alive for 30 seconds
    sleep(10);
    
    // Send some data
    const char* msg = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    send(sockfd, msg, strlen(msg), 0);
    
    // Read response (optional)
    char buffer[1024];
    recv(sockfd, buffer, sizeof(buffer), 0);
    
    close(sockfd);
    std::cout << "Connection " << id << " closed" << std::endl;
}

int main(){
    const int NUM_CONNECTIONS = 150;
    const int PORT = 8080;
    
    std::vector<pid_t> children;
    
    std::cout << "Starting siege with " << NUM_CONNECTIONS << " persistent connections" << std::endl;
    
    for (int i = 0; i < NUM_CONNECTIONS; i++){
        pid_t pid = fork();
        
        if (pid == 0) {  // Child process
            createConnection(i, "127.0.0.1", PORT);
            _exit(0);
            
        } else if (pid > 0) {  // Parent process
            children.push_back(pid);
            usleep(10000);  // Small delay between connections (10ms)
            
        } else {  // Fork failed
            perror("fork failed");
            break;
        }
    }
    
    std::cout << "All connections launched. Waiting..." << std::endl;
    
    // Wait for all children
    for (pid_t child : children) {
        int status;
        waitpid(child, &status, 0);
    }
    
    std::cout << "Siege completed." << std::endl;
    return 0;
}
