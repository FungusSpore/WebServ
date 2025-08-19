#include "MiniHttpResponse.hpp"
#include "MiniHttpUtils.hpp"
#include "Socket.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <strings.h>

MiniHttpResponse::MiniHttpResponse(WebServer& server, MiniHttpRequest& request, Socket& socket)
	: _server(server), _serverBlock(0), _locationBlock(0), _request(request), _socket(socket), _statusCode(-1), _statusMessage(""), _body(""), _headers() {}

MiniHttpResponse::~MiniHttpResponse() {}

// ===================================================================
// CORE CONFIGURATION & VALIDATION
// ===================================================================

ServerKey& MiniHttpResponse::createServerKey() {
	ServerKey& key = _socket.getServerKey();

	const std::multimap<std::string, std::string>& headers = _request.getHeaders();
	std::multimap<std::string, std::string>::const_iterator hostIt = headers.find("Host");
	
	std::string hostHeader;
	if (hostIt != headers.end()) {
		hostHeader = hostIt->second;
	}

	if (hostHeader.empty()) {
		// should it throw an error or return a default key?
		throw std::runtime_error("Host header is missing in the request");
	}

	size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos) {
		key._serverName = hostHeader.substr(0, colonPos);
	} else {
		key._serverName = hostHeader;
	}

	return key;
}

bool MiniHttpResponse::validateServerConf() {
	ServerKey& key = createServerKey();

	_serverBlock = _server.matchServer(key);
	if (!_serverBlock) {
		_statusCode = 500;
		return false;
	}
	
	_locationBlock = _serverBlock->matchLocation(_request.getPath());
	if (!_locationBlock) {
		_statusCode = 404;
		return false;
	}

	if (_locationBlock->getAllowedMethods().find(ft_strToHttpMethod(_request.getMethod())) == _locationBlock->getAllowedMethods().end()) {
		_statusCode = 405;
		return false;
	}
	return true;
}

// ===================================================================
// ERROR HANDLING
// ===================================================================

void MiniHttpResponse::setErrorStatus() {
	if (_statusCode < 100 || _statusCode >= 600) {
		_statusCode = 500;
	}
	if (_statusMessage.empty()) {
		_statusMessage = getStatusCodeMsg(_statusCode);
	}
}

void MiniHttpResponse::setParseErrorResponse(int statusCode) {
	_statusCode = statusCode;
	parseErrorResponse();
}

void MiniHttpResponse::defaultErrorResponse() {
	_body = "<!DOCTYPE html>\n<html>\n<head>\n"
			"    <meta charset=\"utf-8\">\n"
			"    <title>" + ft_toString(_statusCode) + " " + _statusMessage + "</title>\n"
			"</head>\n<body>\n"
			"    <h1>" + ft_toString(_statusCode) + " " + _statusMessage + "</h1>\n"
			"    <p>Default Error Page.</p>\n"
			"</body>\n</html>\n";

	_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
	_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
}

void MiniHttpResponse::parseErrorResponse() {
	if (_statusCode < 100 || _statusCode >= 600) {
		_statusCode = 500;
	}
	if (_statusMessage.empty()) {
		_statusMessage = getStatusCodeMsg(_statusCode);
	}

	const std::string* errorPagePath = _serverBlock->getErrorPagePath(_statusCode);
	if (errorPagePath) {
		const Location* errorLocation = _serverBlock->matchLocation("/error_pages/");
		std::string fsPath;
		
		if (!errorLocation) {
			if (!pathExists(*errorPagePath)) {
				std::cout << "Error page path does not exist: " << *errorPagePath << std::endl;
				defaultErrorResponse();
				return;
			}
			fsPath = *errorPagePath;
		} else {
			fsPath = genfsPath(errorLocation, *errorPagePath);
			if (!pathExists(fsPath)) {
				defaultErrorResponse();
				return;
			}
		}

		loadBody(fsPath);
		_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
		_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
	} else {
		defaultErrorResponse();
	}
}

// ===================================================================
// HEADER MANAGEMENT
// ===================================================================

void MiniHttpResponse::parseDefaultHeader()
{
	std::vector<std::pair<std::string, std::string> > defaults;

	if (!hasHeader("Server"))
		defaults.push_back(std::make_pair("Server", "Prophet/1.0"));

	if (!hasHeader("Date"))
		defaults.push_back(std::make_pair("Date", getCurrentTime()));

	if (!hasHeader("Content-Type"))
		defaults.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));

	if (!hasHeader("Content-Length"))
		defaults.push_back(std::make_pair("Content-Length", "0"));

	if (!_socket.keepAlive) {
		defaults.push_back(std::make_pair("Connection", "close"));
	}

	_headers.insert(_headers.begin(), defaults.begin(), defaults.end());
}

bool MiniHttpResponse::hasHeader(const std::string& key) const
{
	for (std::vector<std::pair<std::string,std::string> >::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it) {
		if (strcasecmp(it->first.c_str(), key.c_str()) == 0)
			return true;
	}
	return false;
}

std::string MiniHttpResponse::getHeaderValue(const std::string& key) const {
	for (std::vector<std::pair<std::string,std::string> >::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it) {
		if (strcasecmp(it->first.c_str(), key.c_str()) == 0)
			return it->second;
	}
	return "";
}

// ===================================================================
// FILE & DIRECTORY OPERATIONS
// ===================================================================

void MiniHttpResponse::loadBody(const std::string& path)
{
	_body.clear();
	_body = getFileContent(path);
}

std::string MiniHttpResponse::genfsPath(const Location* locationBlock, const std::string& path)
{
	std::string fsPath;

	if (locationBlock->getPathMode() == ROOT) {
		fsPath = normalizeUnderRoot(locationBlock->getPath(), path);
	} else if (locationBlock->getPathMode() == ALIAS) {
		std::string requestPath = path;
		std::string locationUri = locationBlock->getUriPath();
		
		if (requestPath.find(locationUri) == 0) {
			std::string remainingPath = requestPath.substr(locationUri.length());
			fsPath = joinPath(locationBlock->getPath(), remainingPath);
		} else {
			fsPath = locationBlock->getPath();
		}
	}

	return fsPath;
}

bool MiniHttpResponse::handleSlashRedirect()
{
	bool methodCode = (_request.getMethod() == "GET" || _request.getMethod() == "HEAD");
	_statusCode = methodCode ? 301 : 308;

	std::string redirectPath = _request.getPath();
	if (!redirectPath.empty() && redirectPath[redirectPath.size() - 1] != '/') {
		redirectPath += '/';
	}

	_headers.push_back(std::make_pair("Location", redirectPath));
	_headers.push_back(std::make_pair("Content-Length", "0"));
	_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
	_body.clear();
	return true;
}

void MiniHttpResponse::handleFileRequest(const std::string& fsPath)
{
	if (_request.getMethod() == "HEAD") {
		_statusCode = 200;
		
		std::string extension = getFileExtension(fsPath);
		std::string mimeType = getMimeType(extension);
		_headers.push_back(std::make_pair("Content-Type", mimeType));
		
		struct stat fileStat;
		if (stat(fsPath.c_str(), &fileStat) == 0) {
			_headers.push_back(std::make_pair("Content-Length", ft_toString(fileStat.st_size)));
		} else {
			_headers.push_back(std::make_pair("Content-Length", "0"));
		}
		
		_body.clear();
		return;
	}

	_body = getFileContent(fsPath);
	if (_body.empty()) {
		return setParseErrorResponse(500);
	}

	_statusCode = 200;
	
	std::string extension = getFileExtension(fsPath);
	std::string mimeType = getMimeType(extension);
	
	_headers.push_back(std::make_pair("Content-Type", mimeType));
	_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
}

std::string MiniHttpResponse::buildAutoIndexBody(const std::string& fsPath) const
{
	std::ostringstream indexBody;
	indexBody << "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">"
			  << "<title>Index of " << _request.getPath() << "</title></head><body>\n"
			  << "<h1>Index of " << _request.getPath() << "</h1>\n<ul>\n";

	DIR* dir = opendir(fsPath.c_str());
	if (!dir) {
		indexBody << "<p>Error opening directory: " << fsPath << "</p>\n"
				  << "</body>\n</html>\n";
		return indexBody.str();
	}
	
	for (dirent* e; (e = readdir(dir)) != NULL;) {
		if (e->d_name[0] == '.')
			continue;
		
		std::string entryName = e->d_name;
		std::string entryPath = _request.getPath();
		if (entryPath.empty() || entryPath[entryPath.size() - 1] != '/')
			entryPath += '/';
		entryPath += entryName;

		bool isDir = isDirectory(joinPath(fsPath, entryName));
		indexBody << "<li><a href=\"" << entryPath << (isDir ? "/" : "") << "\">"
				  << entryName << (isDir ? "/" : "") << "</a></li>\n";
	}

	closedir(dir);
	indexBody << "    </ul>\n</body>\n</html>\n";
	return indexBody.str();
}

std::vector<std::string> MiniHttpResponse::createCgiEnv()
{
	std::vector<std::string> envVars;
	
	envVars.push_back("SERVER_SOFTWARE=ProphetServer/1.0");
	envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVars.push_back("REQUEST_METHOD=" + _request.getMethod());
	
	// Request URI, script name, and path info need tweaking
	std::string requestUri = _request.getPath();
	envVars.push_back("REQUEST_URI=" + requestUri);
	
	std::string scriptName = _locationBlock->getUriPath();
	std::string pathInfo = "";
	
	if (requestUri.length() > scriptName.length()) {
		pathInfo = requestUri.substr(scriptName.length());
	}
	
	envVars.push_back("SCRIPT_NAME=" + scriptName);
	if (!pathInfo.empty()) {
		envVars.push_back("PATH_INFO=" + pathInfo);
	}
	
	size_t questionPos = requestUri.find('?');
	if (questionPos != std::string::npos) {
		std::string queryString = requestUri.substr(questionPos + 1);
		envVars.push_back("QUERY_STRING=" + queryString);
	} else {
		envVars.push_back("QUERY_STRING=");
	}
	
	const std::multimap<std::string, std::string>& headers = _request.getHeaders();
	std::multimap<std::string, std::string>::const_iterator hostIt = headers.find("Host");
	if (hostIt != headers.end()) {
		std::string host = hostIt->second;
		size_t colonPos = host.find(':');
		if (colonPos != std::string::npos) {
			envVars.push_back("SERVER_NAME=" + host.substr(0, colonPos));
			envVars.push_back("SERVER_PORT=" + host.substr(colonPos + 1));
		} else {
			envVars.push_back("SERVER_NAME=" + host);
			envVars.push_back("SERVER_PORT=80");
		}
	}
	
	envVars.push_back("REMOTE_ADDR=" + _socket.getServerKey()._ip);
	envVars.push_back("REMOTE_PORT=" + _socket.getServerKey()._port);
	envVars.push_back("REMOTE_HOST=" + _socket.getServerKey()._serverName);

	if (_request.getMethod() == "POST") {
		std::multimap<std::string, std::string>::const_iterator contentTypeIt = headers.find("Content-Type");
		if (contentTypeIt != headers.end()) {
			envVars.push_back("CONTENT_TYPE=" + contentTypeIt->second);
		}
		
		std::multimap<std::string, std::string>::const_iterator contentLengthIt = headers.find("Content-Length");
		if (contentLengthIt != headers.end()) {
			envVars.push_back("CONTENT_LENGTH=" + contentLengthIt->second);
		} else {
			envVars.push_back("CONTENT_LENGTH=" + ft_toString(_request.getBody().size()));
		}
	}
	
	for (std::multimap<std::string, std::string>::const_iterator it = headers.begin(); 
		 it != headers.end(); ++it) {
		std::string headerName = "HTTP_" + it->first;
		for (size_t i = 0; i < headerName.length(); ++i) {
			if (headerName[i] == '-') {
				headerName[i] = '_';
			} else {
				headerName[i] = std::toupper(headerName[i]);
			}
		}
		envVars.push_back(headerName + "=" + it->second);
	}
	
	return envVars;
}

// ===================================================================
// CONTENT HANDLERS
// ===================================================================

void MiniHttpResponse::handleRedirection()
{
	_statusCode = _locationBlock->getRedirectionStatusCode();
	
	if (_statusCode < 300 || _statusCode >= 400) {
		if (_statusCode < 100 || _statusCode >= 600) {
			return setParseErrorResponse(500);
		}
		
		const std::string& redirBody = _locationBlock->getAddress();
		_body = "<!DOCTYPE html>\n<html>\n<head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <p>" + redirBody + "</p>\n"
				"</body>\n</html>\n";
		
		_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
		_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
		return;
	}

	const std::string& redirAddr = _locationBlock->getAddress();
	if (redirAddr.empty()) {
		return setParseErrorResponse(500);
	}
	
	_headers.push_back(std::make_pair("Location", redirAddr));
	
	// Add caching headers for temporary redirects
	if (_statusCode == 302 || _statusCode == 307) {
		_headers.push_back(std::make_pair("Cache-Control", "no-cache, no-store, must-revalidate"));
		_headers.push_back(std::make_pair("Pragma", "no-cache"));
		_headers.push_back(std::make_pair("Expires", "0"));
	}

	if (_request.getMethod() != "HEAD") {
		std::string reason = getStatusCodeMsg(_statusCode);
		_body = "<!DOCTYPE html>\n<html>\n<head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <title>" + ft_toString(_statusCode) + " " + reason + "</title>\n"
				"</head>\n<body>\n"
				"    <h1>" + ft_toString(_statusCode) + " " + reason + "</h1>\n"
				"    <p>The document has moved to <a href=\"" + redirAddr + "\">" + redirAddr + "</a>.</p>\n"
				"</body>\n</html>\n";

		_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
		_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
	} else {
		_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
		_headers.push_back(std::make_pair("Content-Length", "0"));
		_body.clear();
	}
}

void MiniHttpResponse::handleCgi()
{
	std::string fsPath = genfsPath(_locationBlock, _request.getPath());
	
	if (!pathExists(fsPath)) {
		return setParseErrorResponse(404);
	}
	
	if (isDirectory(fsPath)) {
		return setParseErrorResponse(403);
	}
	
	// using shebang cgi so script must be executable
	if (access(fsPath.c_str(), X_OK) != 0) {
		return setParseErrorResponse(403);
	}
	
	_socket.cgiPath = fsPath;
	_socket.cgiEnvs = createCgiEnv();
	_socket.isCgi = true;
	
	_statusCode = 200;
	_statusMessage = getStatusCodeMsg(_statusCode);
}

void MiniHttpResponse::handleAutoIndex()
{
	std::string fsPath = genfsPath(_locationBlock, _request.getPath());

	if (!pathExists(fsPath)) {
		return setParseErrorResponse(404);
	}

	if (isDirectory(fsPath)) {
		if (isSlashRedirect(_request.getPath()) && handleSlashRedirect()) {
			return;
		}

		const std::vector<std::string>& indexFiles = _locationBlock->getIndexFiles();
		if (!indexFiles.empty()) {
			for (std::vector<std::string>::const_iterator it = indexFiles.begin(); 
				 it != indexFiles.end(); ++it) {
				std::string indexPath = joinPath(fsPath, *it);
				if (isFile(indexPath)) {
					fsPath = indexPath;
					break;
				}
			}
		}

		_body.clear();
		if (_request.getMethod() != "HEAD") {
			_body = buildAutoIndexBody(fsPath);
			if (_body.empty()) {
				return setParseErrorResponse(403);
			}
		}

		_statusCode = 200;
		_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
		_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
		return;
	}
	
	if (isFile(fsPath)) {
		handleFileRequest(fsPath);
	}
}

void MiniHttpResponse::handleStaticFile()
{
	std::string fsPath = genfsPath(_locationBlock, _request.getPath());

	if (!pathExists(fsPath)) {
		return setParseErrorResponse(404);
	}

	if (isDirectory(fsPath)) {
		if (isSlashRedirect(_request.getPath()) && handleSlashRedirect()) {
			return;
		}

		std::string indexFile;
		const std::vector<std::string>& configuredIndexFiles = _locationBlock->getIndexFiles();
		
		if (!configuredIndexFiles.empty()) {
			for (std::vector<std::string>::const_iterator it = configuredIndexFiles.begin(); 
				 it != configuredIndexFiles.end(); ++it) {
				std::string indexPath = joinPath(fsPath, *it);
				if (isFile(indexPath)) {
					indexFile = indexPath;
					break;
				}
			}
		} else {
			indexFile = getDefaultIndexFile(fsPath);
		}
		
		if (indexFile.empty()) {
			return setParseErrorResponse(403);
		}
		fsPath = indexFile;
	}

	if (!isFile(fsPath)) {
		return setParseErrorResponse(404);
	}

	handleFileRequest(fsPath);
}

void MiniHttpResponse::handleProxyPass()
{
	// Proxy pass implementation placeholder
	setParseErrorResponse(501); // Not Implemented
}

// ===================================================================
// MAIN PROCESSING METHODS
// ===================================================================

void MiniHttpResponse::parseResponse()
{
	std::cout << "Validating server configuration..." << std::endl;
	if (!validateServerConf()) {
		std::cout << "Server configuration validation failed with status code: " << _statusCode << std::endl;
		return parseErrorResponse();
	}
	std::cout << "Server configuration validated successfully." << std::endl;

	// Route to appropriate handler based on location mode
	switch (_locationBlock->getLocationMode()) {
		case REDIRECTION:
			handleRedirection();
			break;
		case CGI:
			handleCgi();
			break;
		case AUTOINDEX:
			handleAutoIndex();
			break;
		case STATIC:
			handleStaticFile();
			break;
		case PROXYPASS:
			handleProxyPass();
			break;
		default:
			setParseErrorResponse(500);
			break;
	}

	setErrorStatus();
	parseDefaultHeader();
}

std::string MiniHttpResponse::buildResponse()
{
	std::ostringstream response;
	
	// Debug output
	std::cout << "\n=== Building Response ===" << std::endl;
	std::cout << "Status Code: " << _statusCode << std::endl;
	std::cout << "Status Message: " << _statusMessage << std::endl;
	std::cout << "Request Method: " << _request.getMethod() << std::endl;
	std::cout << "Request Path: " << _request.getPath() << std::endl;

	// Ensure valid status code and message
	if (_statusCode < 100 || _statusCode >= 600) {
		_statusCode = 500;
		_statusMessage = getStatusCodeMsg(_statusCode);
	}
	if (_statusMessage.empty()) {
		_statusMessage = getStatusCodeMsg(_statusCode);
	}
	
	// Build HTTP response
	response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
	
	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it) {
		response << it->first << ": " << it->second << "\r\n";
	}
	
	response << "\r\n";
	
	// maybe can remove HEAD and just check if body not empty add Content-Length
	if (_request.getMethod() != "HEAD" && !_body.empty()) {
		// if its not a HEAD request, append the body do i need to add a Content-Length header?
		if (!hasHeader("Content-Length")) {
			_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
		}
		response << _body;
	}
	
	std::cout << "\n\nDebug Response:\n" << response.str() << "\n\nEnd Debug.\n" << std::endl;
	return response.str();
}

