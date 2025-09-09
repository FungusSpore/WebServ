#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <set>
#include <map>
#include <vector>

class Server;
class Node;

// Only one LocationMode can be set in a single Location block
enum LocationMode {
	DEFAULT_LOCATIONMODE,
	STATIC,
	AUTOINDEX,
	REDIRECTION,
	PROXYPASS,
	CGI,
	REMOVE
};

enum PathMode {
	DEFAULT_PATHMODE,
	ROOT,
	ALIAS
};

enum HttpMethod {
	GET,
	POST,
	DELETE,
	HEAD
};

class Location {
private:
	std::string _uriPath;
	std::set<HttpMethod> _allowedMethods;
	PathMode _pathMode;
	std::string _path;
	LocationMode _locationMode;
	bool _isAutoindexOn;
	bool _isCgiOn;
	std::string _address;
	long _clientMaxBodySize;
	long _clientTimeout;
	std::vector<std::string> _indexFiles;
	std::map<int, std::string> _errorPages;
	int _redirectionStatusCode;
	bool _isAllowedMethodsSet;
	bool _isClientMaxBodySizeSet;
	bool _isClientTimeoutSet;
	bool _isRootSet;
	bool _isAliasSet;
	bool _isAutoindexSet;
	bool _isProxyPassSet;
	bool _isRedirectionSet;
	bool _isCgiSet;

	static void handleRootDir(Location& location, const Node& node);
	static void handleAliasDir(Location& location, const Node& node);
	static void handleErrorPageDir(Location& location, const Node& node);
	static void handleAllowedMethodDir(Location& location, const Node& node);
	static void handleIndexDir(Location& location, const Node& node);
	static void handleCgiDir(Location& location, const Node& node);
	static void handleAutoindexDir(Location& location, const Node& node);
	static void handleProxyPassDir(Location& location, const Node& node);
	static void handleRedirectionDir(Location& location, const Node& node);
	static void handleClientMaxBodySizeDir(Location& location, const Node& node);
	static void handleClientTimeoutDir(Location& location, const Node& node);
	static void handleUnsupportedDir(const Node& node);
	static void handleUnknownDir(const Node& node);

public:
	Location(const Server& server, const Node& locationNode);
	Location(const Location& other);
	Location& operator=(const Location& other);
	~Location();

	// GETTERS
	const std::string& getUriPath() const;
	const std::set<HttpMethod>& getAllowedMethods() const;
	PathMode getPathMode() const;
	const std::string& getPath() const;
	LocationMode getLocationMode() const;
	bool getIsAutoindexOn() const;
	bool getIsCgiOn() const;
	const std::string& getAddress() const;
	long getClientMaxBodySize() const;
	long getClientTimeout() const;
	const std::vector<std::string>& getIndexFiles() const;
	const std::map<int, std::string>& getErrorPages() const;
	int getRedirectionStatusCode() const;
	bool getIsAllowedMethodsSet() const;
	bool getIsClientMaxBodySizeSet() const;
	bool getIsClientTimeoutSet() const;
	bool getIsRootSet() const;
	bool getIsAliasSet() const;
	bool getIsAutoindexSet() const;
	bool getIsProxyPassSet() const;
	bool getIsRedirectionSet() const;
	bool getIsCgiSet() const;

	// SETTERS
	void setUriPath(const std::string& uriPath);
	void setAllowedMethods(const std::set<HttpMethod>& newAllowedMethods);
	void setPathMode(PathMode mode);
	void setPath(const std::string& path);
	void setLocationMode(LocationMode mode);
	void setIsAutoindexOn();
	void setIsCgiOn();
	void setAddress(const std::string& address);
	void setClientMaxBodySize(long clientMaxBodySize);
	void setClientTimeout(long clientTimeout);
	void setIndexFiles(const std::vector<std::string>& newIndexFiles);
	void setErrorPages(const std::map<int, std::string>& newErrorPages);
	void setRedirectionStatusCode(int redirectionStatusCode);
	void setIsAllowedMethods();
	void setIsClientMaxBodySize();
	void setIsClientTimeout();
	void setIsRoot();
	void setIsAlias();
	void setIsAutoindex();
	void setIsProxyPass();
	void setIsRedirection();
	void setIsCgi();

	// PUBLIC UTILS
	const std::string* getErrorPagePath(int statusCode) const;
	void addErrorPage(int code, const std::string& errorPagePath);
};

#endif // !LOCATION_HPP
