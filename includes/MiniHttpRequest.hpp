#ifndef MINIHTTPREQUEST_HPP
#define MINIHTTPREQUEST_HPP

#include <iostream>
#include <map>

class MiniHttpRequest {
private:
	std::string _method;
	std::string _path;
	std::string _version;
	std::multimap<std::string, std::string> _headers;
	std::string _body;
	int _socket_fd;

	void loadHeader(std::string& request);
	void parseHeader(const std::string& request);
	std::string getHeaderValue(const std::string& key) const;
	void getBodyType(bool& isChunked, long long& contentLength);
	void parseTrailer(std::string& chunk);
	void loadBody(bool isChunked, long long contentLength);

public:
	MiniHttpRequest(int socket_fd);
	MiniHttpRequest(const MiniHttpRequest& other);
	MiniHttpRequest& operator=(const MiniHttpRequest& other);
	~MiniHttpRequest();

	// Parses the HTTP request from the socket
	void parseRequest();

	// Getters
	const std::string& getMethod() const;
	const std::string& getPath() const;
	const std::string& getVersion() const;
	const std::multimap<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;
	int getSocketFd() const;

	// Setters
	void setMethod(const std::string& method);
	void setPath(const std::string& path);
	void setVersion(const std::string& version);
	void addHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setSocketFd(int socket_fd);
};

#endif
