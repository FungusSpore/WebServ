#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP


#include <vector>
#include <map>
#include <string>
#include <set>
#include <fstream>
// root appends path with request
// /img/cat.jpg -> /var/www/html/img/cat.jpg
//
// alias replace matched location with path
// /img/cat.jpg -> /var/www/html/cat.jpg

// enum LocationMode {
// STATIC,
// AUTOINDEX,
// PROXYPASS,
// REDIRECTION,
// };

enum LocationMode {
    REDIRECTION,   // Highest priority
    CGI,
    // UPLOAD,
    AUTOINDEX,
    STATIC,
    PROXYPASS      // Only used if implementing reverse proxy
};

enum PathMode {
ROOT,
ALIAS,
};

enum HttpMethod {
GET,
POST,
// PUT,
HEAD,
// OPTIONS,
// PATCH,
// TRACE,
// CONNECT,
// // DELETE is not commonly used in web servers, but can be included if needed
DELETE,
};

class Location {
private:
	// get complete path based on root or alias?
	std::string _uriPath;
	std::set<HttpMethod> _allowedMethods;
	bool _hasCustomMethods;
	LocationMode _locationMode;
	std::string _path;
	std::string _address;
	PathMode _pathMode;
	bool _hasRootPath;
	long _clientMaxBodySize;
	bool _hasClientMaxBodySize;
	std::map<int, std::string> _errorPages;
	bool _hasErrorPages;
	std::vector<std::string> _indexFiles;
	std::string _proxyPassAddress;
	int _redirectionStatusCode;
	std::string _redirectionAddress;

public:
	Location();
	// Location(); // pass in AST Tree?
	Location(const Location& other);
	Location& operator=(const Location& other);
	~Location();

	// Setters for dummy implementation
	void setUriPath(const std::string& uriPath);
	void setLocationMode(LocationMode mode);
	void setPath(const std::string& path);
	void setPathMode(PathMode mode);
	void setAllowedMethods(const std::set<HttpMethod>& methods);
	void setIndexFiles(const std::vector<std::string>& indexFiles);
	void setAddress(const std::string& address);
	void setRedirectionStatusCode(int statusCode);

	const std::string& getUriPath() const;

	// getters for other members
	const std::set<HttpMethod>& getAllowedMethods() const;
	// bool hasCustomMethods() const;
	LocationMode getLocationMode() const;
	const std::string& getPath() const;
	PathMode getPathMode() const;
	// bool hasRootPath() const;
	long getClientMaxBodySize() const;
	bool hasClientMaxBodySize() const;
	const std::map<int, std::string>& getErrorPages() const;
	// bool hasErrorPages() const;
	const std::vector<std::string>& getIndexFiles() const;
	// const std::string& getProxyPassAddress() const;
	int getRedirectionStatusCode() const;
	// const std::string& getRedirectionAddress() const;
	const std::string& getAddress() const;
};

struct ServerKey {
	std::string ip;
	std::string port;
	std::string serverName;
};

class Server {
private:
	std::string _ip;
	std::string	_port;
	std::string	_serverName;
	std::string _rootPath;
	std::set<HttpMethod> _allowedMethods;
	long _clientMaxBodySize;
	std::map<int, std::string> _errorPages;
	std::vector<Location> _locations;
	std::vector<std::string> _indexFiles;

public:
	Server();
	// Server(); //pass in AST tree?
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();
	const Location* matchLocation(const std::string& target) const;
	const std::string* getErrorPagePath(int statusCode) const;
};

class WebServer {
private:
	std::vector<std::string> _ports;
	std::map<ServerKey, Server> _serverMap;

public:
	WebServer();
	WebServer(const char* filename);
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

	const Server* matchServer(const ServerKey& key) const;
	const std::vector<std::string>& getPorts() const;
};

bool operator<(const ServerKey& lhs, const ServerKey& rhs);

#endif
