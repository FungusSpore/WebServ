#ifndef SERVER_HPP
#define SERVER_HPP

#include "DirChecker.hpp"
#include "Location.hpp"

#define DEFAULT_CLIENT_MAX_BODY_SIZE 1024000 // in bytes
#define DEFAULT_MAX_HEADER_SIZE 8192 // in bytes
#define DEFAULT_CLIENT_TIMEOUT 20 // in seconds

struct ServerKey {
	std::string _ip;
	std::string _port;
	std::string _serverName;

	ServerKey(const std::string& ip, const std::string& port, const std::string& serverName);
};

// last one wins for index files and error_pages.
// no duplicate allowed for others.
class Server {
private:
	std::string _ip;
	std::string	_port;
	std::string	_serverName;
	std::string _rootPath;
	std::set<HttpMethod> _allowedMethods;
	long _clientMaxBodySize;
	long _clientTimeout;
	std::vector<std::string> _indexFiles;
	std::map<int, std::string> _errorPages;
	std::vector<Location> _locations;
	std::set<std::string> _locationTargets;
	bool _isListenSet;
	bool _isServerNameSet;
	bool _isRootPathSet;
	bool _isAllowedMethodsSet;
	bool _isClientMaxBodySizeSet;
	bool _isClientTimeoutSet;

	static void handleListenDir(Server& server, const Node& listenNode);
	static void handleServerNameDir(Server& server, const Node& serverNameNode);
	static void handleErrorPageDir(Server& server, const Node& errorPageNode);
	static void handleAllowedMethodDir(Server& server, const Node& allowedMethodNode);
	static void handleIndexDir(Server& server, const Node& indexNode);
	static void handleClientMaxBodySizeDir(Server& server, const Node& clientMaxBodySizeNode);
	static void handleClientTimeoutDir(Server& server, const Node& clientTimeoutNode);
	static void handleUnsupportedDir(const Node& node);
	static void handleUnknownDir(const Node& unknownNode);
	static void handleLocationDir(Server& server, const Node& node);

	// PRIVATE UTILS
	static void checkServerBlock(const Node& serverNode);
	static void assignRootPath(Server& server, const Node& serverNode);
	static void assignDefaultErrorPages(Server& server);
	static std::set<HttpMethod> getDefaultHttpMethods();

public:
	Server(const Node& serverNode);
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

	// GETTERS
	const std::string& getIp() const;
	const std::string& getPort() const;
	const std::string& getServerName() const;
	const std::string& getRootPath() const;
	const std::set<HttpMethod>& getAllowedMethods() const;
	long getClientMaxBodySize() const;
	long getClientTimeout() const;
	const std::vector<std::string>& getIndexFiles() const;
	const std::map<int, std::string>& getErrorPages() const;
	const std::vector<Location>& getLocations() const;
	const std::set<std::string>& getLocationTargets() const;
	bool getIsListenSet() const;
	bool getIsServerNameSet() const;
	bool getIsRootPathSet() const;
	bool getIsAllowedMethodsSet() const;
	bool getIsClientMaxBodySizeSet() const;
	bool getIsClientTimeoutSet() const;

	// SETTERS
	void setIp(const std::string& ip);
	void setPort(const std::string& port);
	void setServerName(const std::string& serverName);
	void setRootPath(const std::string& path);
	void setAllowedMethods(const std::set<HttpMethod>& newAllowedMethods);
	void setClientMaxBodySize(long clientMaxBodySize);
	void setClientTimeout(long clientTimeout);
	void setIndexFiles(const std::vector<std::string>& newIndexFiles);
	void setErrorPages(const std::map<int, std::string>& newErrorPages);
	void setLocations(const std::vector<Location>& newLocations);
	void setIsListen();
	void setIsServerName();
	void setIsRootPath();
	void setIsAllowedMethods();
	void setIsClientMaxBodySize();
	void setIsClientTimeout();

	// PUBLIC UTILS
	const Location* matchLocation(const std::string& target) const;
	const std::string* getErrorPagePath(int statusCode) const;
	void addErrorPage(int code, const std::string& errorPagePath);
	void addLocation(const Location& location);
	void addLocationTarget(const std::string& locationTarget);
};

bool operator<(const ServerKey& lhs, const ServerKey& rhs);

#endif // !SERVER_HPP
