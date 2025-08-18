#ifndef MINIHTTPRESPONSE_HPP
#define MINIHTTPRESPONSE_HPP

#include <iostream>
#include <vector>
#include <string>
#include "MiniHttpRequest.hpp"
#include "WebServer.hpp"

class MiniHttpResponse {
private:
	// Core dependencies
	WebServer& _server;
	const Server* _serverBlock;
	const Location* _locationBlock;
	MiniHttpRequest& _request;
	Socket& _socket;

	// Response data
	int _statusCode;
	std::string _statusMessage;
	std::string _body;
	std::vector<std::pair<std::string, std::string> > _headers;

	// === Core Configuration & Validation ===
	ServerKey& createServerKey();
	bool validateServerConf();

	// === Error Handling ===
	void setErrorStatus();
	void setParseErrorResponse(int statusCode);
	void parseErrorResponse();
	void defaultErrorResponse();

	// === Header Management ===
	void parseDefaultHeader();
	bool hasHeader(const std::string& key) const;

	// === Content Handlers ===
	void handleRedirection();
	void handleCgi();
	void handleAutoIndex();
	void handleStaticFile();
	void handleProxyPass();

	// === File & Directory Operations ===
	void loadBody(const std::string& path);
	void handleFileRequest(const std::string& fsPath);
	std::string buildAutoIndexBody(const std::string& fsPath) const;
	std::string genfsPath(const Location* locationBlock, const std::string& path);
	bool handleSlashRedirect();

private:
	// Prevent copying (C++98 style)
	MiniHttpResponse(const MiniHttpResponse& other);
	MiniHttpResponse& operator=(const MiniHttpResponse& other);

public:
	MiniHttpResponse(WebServer& server, MiniHttpRequest& request, Socket& socket);
	~MiniHttpResponse();

	// Main interface
	void parseResponse();
	std::string buildResponse();
};

#endif
