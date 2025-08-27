#ifndef MINIHTTPREQUEST_HPP
#define MINIHTTPREQUEST_HPP

#include <iostream>
#include <map>
#include <vector>

struct Socket;

class MiniHttpRequest {
private:
	Socket& _socket;
	std::string _method, _path, _version, _trailer;
	std::vector<char> _buffer, _body;
	std::multimap<std::string, std::string> _headers;
	// int _socket_fd;
	bool _isHeaderLoaded;
	int _isErrorCode;

	bool loadHeader();
	void parseHeader();
	std::string getHeaderValue(const std::string& key) const;
	void getBodyType(bool& isChunked, long long& contentLength);
	void parseTrailer();
	bool loadBody(bool isChunked, long long contentLength);

	// Copy constructor and assignment operator are declared private and not implemented
	// to prevent copying objects with reference members (C++98 idiom)
	MiniHttpRequest(const MiniHttpRequest& other);
	MiniHttpRequest& operator=(const MiniHttpRequest& other);

public:
	MiniHttpRequest(Socket& socket);
	~MiniHttpRequest();

	// Parses the HTTP request from the socket
	bool parseRequest();

	// clean up resources
	void clearRequest();

	// Getters
	Socket& getSocket() const;
	const std::string& getMethod() const;
	const std::string& getPath() const;
	const std::string& getVersion() const;
	const std::vector<char>& getBody() const;
	const std::string& getTrailer() const;
	const std::multimap<std::string, std::string>& getHeaders() const;
	int getErrorCode() const;

	// Setters
	// void setMethod(const std::string& method);
	// void setPath(const std::string& path);
	// void setVersion(const std::string& version);
	// void setBody(const std::string& body);
	// void setTrailer(const std::string& trailer);
	void addHeader(const std::string& key, const std::string& value);
};

#endif
