#include <iostream>
#include <sstream>
#include <strings.h>
#include "MiniHttpResponse.hpp"
#include "MiniHttpUtils.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

MiniHttpResponse::MiniHttpResponse(WebServer& server, MiniHttpRequest& request, int socket_fd)
	: _server(server), _serverBlock(0), _locationBlock(0), _request(request), _socket_fd(socket_fd), _statusCode(-1), _statusMessage(""), _body("") {}

MiniHttpResponse::MiniHttpResponse(const MiniHttpResponse& other)
	: _server(other._server), _serverBlock(other._serverBlock), _locationBlock(other._locationBlock), 
	  _request(other._request), _socket_fd(other._socket_fd), _statusCode(other._statusCode), 
	  _statusMessage(other._statusMessage), _body(other._body), _headers(other._headers) {}

MiniHttpResponse& MiniHttpResponse::operator=(const MiniHttpResponse& other) {
	if (this != &other) {
		_server = other._server;
		_serverBlock = other._serverBlock;
		_locationBlock = other._locationBlock;
		_request = other._request;
		_socket_fd = other._socket_fd;
		_statusCode = other._statusCode;
		_statusMessage = other._statusMessage;
		_body = other._body;
		_headers = other._headers;
	}
	return *this;
}

MiniHttpResponse::~MiniHttpResponse() {
	// Destructor implementation
}

ServerKey MiniHttpResponse::createServerKey() {
	ServerKey key;

	key.ip = ""; // Default IP if not specified
	std::string hostHeader = _request.getHeaders().find("Host")->second;

	if (hostHeader.empty()) {
		throw std::runtime_error("Host header is missing in the request");
	}

	size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos) {
		key.serverName = hostHeader.substr(0, colonPos);
		key.port = hostHeader.substr(colonPos + 1);
	} else {
		key.serverName = hostHeader;
		key.port = "80"; // Default port if not specified
	}

	return key;
}

bool MiniHttpResponse::validateServerConf() {
	// Validate the server configuration based on the key
	ServerKey key = createServerKey();
	_serverBlock = _server.matchServer(key);
	if (!_serverBlock) {
		_statusCode = 500; // Internal Server Error
		return false;
	}
	_locationBlock = _serverBlock->matchLocation(_request.getPath());
	if (!_locationBlock) {
		_statusCode = 404; // Not Found
		return false;
	}

	// Check allowed methods
	if (_locationBlock->getAllowedMethods().find(ft_strToHttpMethod(_request.getMethod())) == _locationBlock->getAllowedMethods().end()) {
		_statusCode = 405;
		return false; // Method Not Allowed
	}
	return true;
}

void MiniHttpResponse::loadBody(const std::string& path) {
	// Load the body from the specified path
	_body.clear();
	_body = getFileContent(path);
	// if (_body.empty()) {
	// 	throw std::runtime_error("Failed to load body from path: " + path);
	// }
}

void MiniHttpResponse::parseErrorCode() {
	// this function should return the error code based on the status code
	// body will have the error page content /var/www/errors/
	if (_statusCode < 400 || _statusCode >= 600) {
		throw std::invalid_argument("Invalid HTTP status code for error response");
	}
	std::string errorPagePath = "/var/www/errors/" + ft_toString(_statusCode) + ".html";
	loadBody(errorPagePath);
	if (_body.empty()) {
		throw std::runtime_error("Error page not found for status code: " + ft_toString(_statusCode));
	}


}

void MiniHttpResponse::parseErrorResponse() {
	// Set default error response if not already set
	if (_statusCode < 100 || _statusCode >= 600) {
		_statusCode = 500; // Internal Server Error
	}
	if (_statusMessage.empty()) {
		_statusMessage = getStatusCodeMsg(_statusCode);
	}

	// Check if there are custome error pages configured
	const std::string* errorPagePath = _serverBlock->getErrorPagePath(_statusCode);
	if (errorPagePath) {
		loadBody(*errorPagePath);
		if (_body.empty()) {
			throw std::runtime_error("Error page not found for status code: " + ft_toString(_statusCode));
		}
	} else {
		// If no custom error page is configured, use a default error message
		_body = "<!DOCTYPE html>\n<html>\n<head>\n";
		_body += "    <meta charset=\"utf-8\">\n";
		_body += "    <title>" + ft_toString(_statusCode) + " " + _statusMessage + "</title>\n";
		_body += "</head>\n<body>\n";
		_body += "    <h1>" + ft_toString(_statusCode) + " " + _statusMessage + "</h1>\n";
		_body += "	<p>An error occurred while processing your request.</p>\n";
		_body += "</body>\n</html>\n";
	}

	// Set default headers
	_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
	_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
}

void MiniHttpResponse::setErrorStatus() {
	// Set a default error status if not already set
	if (_statusCode < 100 || _statusCode >= 600) {
		// set to 500 Internal Server Error
		_statusCode = 500;
	}
	if (_statusMessage.empty()) {
		_statusMessage = getStatusCodeMsg(_statusCode);
	}
}

void MiniHttpResponse::setParseErrorResponse(int statusCode) {
	// Set the status code and message for parse errors
	_statusCode = statusCode;
	parseErrorResponse();
}

void MiniHttpResponse::parseDefaultHeader() {
    // Prepare defaults
    std::vector<std::pair<std::string, std::string> > defaults;

    if (!hasHeader("Server"))
        defaults.push_back(std::make_pair("Server", "Prophet/1.0"));

    if (!hasHeader("Date"))
        defaults.push_back(std::make_pair("Date", getCurrentTime()));

    if (!hasHeader("Content-Type"))
        defaults.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));

    if (!hasHeader("Content-Length"))
        defaults.push_back(std::make_pair("Content-Length", "0"));

    if (!hasHeader("Connection")) {
        defaults.push_back(std::make_pair("Connection", "close"));
	} else {
		// block for if client wnats to keep connection open and have content-length
	}

    // Insert defaults at the front, preserving their relative order
    _headers.insert(_headers.begin(), defaults.begin(), defaults.end());
}

bool MiniHttpResponse::hasHeader(const std::string& key) const {
	for (std::vector<std::pair<std::string,std::string> >::const_iterator it = _headers.begin();
	it != _headers.end(); ++it) {
		if (strcasecmp(it->first.c_str(), key.c_str()) == 0)
			return true;
	}
	return false;
}

std::string MiniHttpResponse::buildAutoIndexBody(const std::string& fsPath) const {
	// This function should build the autoindex body based on the location block
	std::ostringstream indexBody;
	indexBody
		<< "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">"
		<< "<title>Index of " << _request.getPath() << "</title></head><body>\n"
		<< "<h1>Index of " << _request.getPath() << "</h1>\n<ul>\n";

	DIR* dir = opendir(fsPath.c_str());
	if (!dir) {
		indexBody << "<p>Error opening directory: " << fsPath << "</p>\n";
		indexBody << "</body>\n</html>\n";
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

void MiniHttpResponse::parseResponse() {
	// Validate server configuration
	if (!validateServerConf()) {
		return parseErrorResponse();
	}



	if (_statusCode == -1) {
		throw std::runtime_error("Status code not set for response");
	}

	// decide handling mode
	if (_locationBlock->getLocationMode() == REDIRECTION) {
		// Handle redirection logic

		_statusCode = _locationBlock->getRedirectionStatusCode();
		if (_statusCode < 300 || _statusCode >= 400) {
			throw std::runtime_error("Invalid redirection status code: " + ft_toString(_statusCode));
		}
		const std::string& redirAddr = _locationBlock->getAddress();
		if (redirAddr.empty()) {
			throw std::runtime_error("Redirection address is empty for status code: " + ft_toString(_statusCode));
		}
		_headers.push_back(std::make_pair("Location", redirAddr));
		
		// optional caching headers if code is 302 or 307
		if (_statusCode == 302 || _statusCode == 307) {
			_headers.push_back(std::make_pair("Cache-Control", "no-cache, no-store, must-revalidate"));
			_headers.push_back(std::make_pair("Pragma", "no-cache"));
			_headers.push_back(std::make_pair("Expires", "0"));
		}

		// Prepare body only for non-HEAD requests
		if (_request.getMethod() != "HEAD") {
			std::string reason = getStatusCodeMsg(_statusCode);
			_body  = "<!DOCTYPE html>\n<html>\n<head>\n";
			_body += "    <meta charset=\"utf-8\">\n";
			_body += "    <title>";
			_body += ft_toString(_statusCode) + " " + reason;
			_body += "</title>\n</head>\n<body>\n";
			_body += "    <h1>";
			_body += ft_toString(_statusCode) + " " + reason;
			_body += "</h1>\n";
			_body += "    <p>The document has moved to <a href=\"";
			_body += redirAddr;
			_body += "\">";
			_body += redirAddr;
			_body += "</a>.</p>\n";
			_body += "</body>\n</html>\n";

			_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
			_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
		} else {
			// HEAD request: no body
			_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
			_headers.push_back(std::make_pair("Content-Length", "0"));
			_body.clear();
		}

	} else if (_locationBlock->getLocationMode() == CGI) {
		// Handle CGI logic here (not implemented in this example)
	// } else if (_locationBlock->getLocationMode() == UPLOAD) {
	// 	// Handle file upload logic here (not implemented in this example)
	} else if (_locationBlock->getLocationMode() == AUTOINDEX) {
		// Handle autoindex logic
		std::string fsPath;

		if (_locationBlock->getPathMode() == ROOT) {
			fsPath = normalizeUnderRoot(_locationBlock->getPath(), _request.getPath());
		} else if (_locationBlock->getPathMode() == ALIAS) {
			// For ALIAS, replace the matched location prefix with the alias path
			std::string requestPath = _request.getPath();
			std::string locationUri = _locationBlock->getUriPath();
			
			if (requestPath.find(locationUri) == 0) {
				std::string remainingPath = requestPath.substr(locationUri.length());
				fsPath = joinPath(_locationBlock->getPath(), remainingPath);
			} else {
				fsPath = _locationBlock->getPath();
			}
		}

		if (!pathExists(fsPath)) {
			return setParseErrorResponse(404);
		}

		if (!isDirectory(fsPath)) {
			return setParseErrorResponse(403); // Forbidden if not a directory
		}

		if (isSlashRedirect(_request.getPath())) {
			// Redirect to the directory with a trailing slash
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
			return;
		}

		// Build the autoindex body
		_body = buildAutoIndexBody(fsPath);
		if (_body.empty()) {
			return setParseErrorResponse(403);
		}

		// Set successful response
		_statusCode = 200;
		_headers.push_back(std::make_pair("Content-Type", "text/html; charset=utf-8"));
		_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
		return;








	} else if (_locationBlock->getLocationMode() == STATIC) {
		// Handle static file serving
		std::string fsPath;

		if (_locationBlock->getPathMode() == ROOT) {
			fsPath = normalizeUnderRoot(_locationBlock->getPath(), _request.getPath());
		} else if (_locationBlock->getPathMode() == ALIAS) {
			// For ALIAS, replace the matched location prefix with the alias path
			std::string requestPath = _request.getPath();
			std::string locationUri = _locationBlock->getUriPath();
			
			if (requestPath.find(locationUri) == 0) {
				std::string remainingPath = requestPath.substr(locationUri.length());
				fsPath = joinPath(_locationBlock->getPath(), remainingPath);
			} else {
				fsPath = _locationBlock->getPath();
			}
		}

		if (!pathExists(fsPath)) {
			return setParseErrorResponse(404);
		}

		if (isDirectory(fsPath)) {
			// Check if URL needs trailing slash redirect
			if (isSlashRedirect(_request.getPath())) {
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
				return;
			}

			// Look for index files
			std::string indexFile;
			const std::vector<std::string>& configuredIndexFiles = _locationBlock->getIndexFiles();
			
			if (!configuredIndexFiles.empty()) {
				// Use configured index files
				for (std::vector<std::string>::const_iterator it = configuredIndexFiles.begin(); 
					 it != configuredIndexFiles.end(); ++it) {
					std::string indexPath = joinPath(fsPath, *it);
					if (isFile(indexPath)) {
						indexFile = indexPath;
						break;
					}
				}
			} else {
				// Use default index files
				indexFile = getDefaultIndexFile(fsPath);
			}
			
			if (indexFile.empty()) {
				// No index file found, return 403 Forbidden or 404 depending on server config
				return setParseErrorResponse(403);
			}
			fsPath = indexFile;
		}

		if (!isFile(fsPath)) {
			return setParseErrorResponse(404);
		}

		// Handle HEAD requests - no body content
		if (_request.getMethod() == "HEAD") {
			_statusCode = 200;
			
			// Get file extension and MIME type
			std::string extension = getFileExtension(fsPath);
			std::string mimeType = getMimeType(extension);
			
			_headers.push_back(std::make_pair("Content-Type", mimeType));
			
			// Get file size for Content-Length
			struct stat fileStat;
			if (stat(fsPath.c_str(), &fileStat) == 0) {
				_headers.push_back(std::make_pair("Content-Length", ft_toString(fileStat.st_size)));
			} else {
				_headers.push_back(std::make_pair("Content-Length", "0"));
			}
			
			_body.clear();
			return;
		}

		// For GET requests, load the file content
		_body = getFileContent(fsPath);
		if (_body.empty()) {
			return setParseErrorResponse(500); // Internal Server Error if file can't be read
		}

		_statusCode = 200;
		
		// Get file extension and set appropriate MIME type
		std::string extension = getFileExtension(fsPath);
		std::string mimeType = getMimeType(extension);
		
		_headers.push_back(std::make_pair("Content-Type", mimeType));
		_headers.push_back(std::make_pair("Content-Length", ft_toString(_body.size())));
		return;

	} else if (_locationBlock->getLocationMode() == PROXYPASS) {
		// Handle reverse proxy logic here (not implemented in this example)
	}


	// set status line
	setErrorStatus();
	const std::string statusLine = "HTTP/1.1 " + ft_toString(_statusCode) + " " + _statusMessage + "\r\n";
	// _headers.insert(_headers.begin(), std::make_pair("StatusLine", statusLine));

	// Set default header based on nginx
	parseDefaultHeader();
}

