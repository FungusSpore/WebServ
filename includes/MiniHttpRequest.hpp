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
	bool _isHeaderLoaded;
	int _isErrorCode;

	// Header Handling
	bool loadHeader();
	void parseHeader();
	std::string getHeaderValue(const std::string& key) const;

	// Body Handling
	void getBodyType(bool& isChunked, long long& contentLength);
	void parseTrailer();
	bool loadBody(bool isChunked, long long contentLength);

	// Disallow copy and assignment
	MiniHttpRequest(const MiniHttpRequest& other);
	MiniHttpRequest& operator=(const MiniHttpRequest& other);

public:
	MiniHttpRequest(Socket& socket);
	~MiniHttpRequest();

	bool parseRequest();
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

	// Setter
	void addHeader(const std::string& key, const std::string& value);
};

#endif
