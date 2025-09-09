#ifndef MINIHTTPRESPONSE_HPP
#define MINIHTTPRESPONSE_HPP

#include <iostream>
#include <vector>
#include <string>
#include "MiniHttpRequest.hpp"
#include "WebServer.hpp"

class MiniHttpResponse {
private:
	WebServer& _server;
	const Server* _serverBlock;
	const Location* _locationBlock;
	MiniHttpRequest& _request;
	Socket& _socket;

	int _statusCode;
	std::string _statusMessage;
	std::vector<char> _body;
	std::vector<std::pair<std::string, std::string> > _headers;
	Cookie* _cookie;

	// === Core Configuration & Validation ===
	ServerKey& createServerKey();
	bool validateServerConf();
	bool validateClientMaxBodySize();
	bool validateHeaderSize();

	// === Error Handling ===
	void setErrorStatus();
	void setParseErrorResponse(int statusCode);
	void parseErrorResponse();
	void defaultErrorResponse();

	// === Header Management ===
	void parseDefaultHeader();
	bool hasHeader(const std::string& key) const;
	std::string getHeaderValue(const std::string& key) const;

	// === Content Handlers ===
	void handleRedirection();
	void handleCgi();
	void handleAutoIndex();
	void handleStaticFile();
	void handleDelete();

	// === File & Directory Operations ===
	void loadBody(const std::string& path);
	void handleFileRequest(const std::string& fsPath);
	std::vector<char> buildAutoIndexBody(const std::string& fsPath) const;
	std::string genfsPath(const Location* locationBlock, const std::string& path);
	bool handleSlashRedirect();
	std::vector<std::string> createCgiEnv();

private:
	MiniHttpResponse(const MiniHttpResponse& other);
	MiniHttpResponse& operator=(const MiniHttpResponse& other);

public:
	MiniHttpResponse(WebServer& server, MiniHttpRequest& request, Socket& socket);
	~MiniHttpResponse();

	void parseResponse();
	std::vector<char> buildResponse();

	// === Cookie ===
	void parseCookie();
};

#endif
