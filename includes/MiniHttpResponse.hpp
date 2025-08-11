#ifndef MINIHTTPRESPONSE_HPP
#define MINIHTTPRESPONSE_HPP

#include <iostream>
#include "MiniHttpRequest.hpp"
// #include "MiniHttpRoute.hpp"
#include "WebServer.hpp"

class MiniHttpResponse {
private:
	WebServer& _server;
	const Server* _serverBlock;
	const Location* _locationBlock;

	MiniHttpRequest& _request;
	int _socket_fd;

	int _statusCode;
	std::string _statusMessage;
	std::string _body;
	std::vector<std::pair<std::string, std::string> > _headers;

	ServerKey createServerKey();
	bool validateServerConf();
	void loadBody(const std::string& path);
	void parseErrorCode();
	void parseErrorResponse();
	void setParseErrorResponse(int statusCode);
	void parseDefaultHeader();
	bool hasHeader(const std::string& key) const;

	std::string buildAutoIndexBody(const std::string& fsPath) const;

	// std::string getBody() const;
	
	void handleRedirection();
	void handleCgi();
	void handleUpload();
	void handleAutoIndex();
	void handleStatic();
	void handleProxyPass();
	

	void setErrorStatus();

public:
	MiniHttpResponse(WebServer& server, MiniHttpRequest& request, int socket_fd);
	// MiniHttpResponse(MiniHttpRoute& route);
	// MiniHttpResponse(MiniHttpRoute& route, int statusCode);
	// MiniHttpResponse(MiniHttpRoute& route, int statusCode, const std::string& _statusMessage);
	MiniHttpResponse(const MiniHttpResponse& other);
	MiniHttpResponse& operator=(const MiniHttpResponse& other);
	~MiniHttpResponse();

	void parseResponse();

	// build a getters for the most common headers to lookup for parseResponse
	// std::multimap<std::string, std::string>& getHeaders() {
	// 	return _headers;
	// }


};

#endif
