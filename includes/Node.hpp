#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <string>
#include <map>

enum ConfigDirective {
	DIR_HTTP,
	DIR_SERVER,
	DIR_LOCATION,
	DIR_LISTEN,
	DIR_SERVER_NAME,
	DIR_ROOT,
	DIR_ALIAS,
	DIR_ERROR_PAGE,
	DIR_ALLOWED_METHOD,
	DIR_INDEX,
	DIR_CGI,
	DIR_AUTOINDEX,
	DIR_PROXY_PASS,
	DIR_REDIRECTION,
	DIR_RETURN,
	DIR_CLIENT_MAX_BODY_SIZE,
	DIR_CLIENT_TIMEOUT,
	DIR_UNKNOWN
};

class Node {
private:
	std::string _name;
	// std::vector<std::string> _args;
	std::vector<Node> _args;
	std::vector<Node> _children;
	int _line;
	int _col;

	static const std::map<std::string, ConfigDirective> _directiveMap;
	static std::map<std::string, ConfigDirective> initDirectiveMap();

public:
	Node();
    Node(const std::string& name, int line, int col);
	Node(const Node& other);
	Node& operator=(const Node& other);
	~Node();

	void setName(const std::string& name);
	void setPosition(int line, int col);
	// void addArguments(const std::string& arg);
	void addArguments(const Node& arg);
	void addChild(const Node& children);
	const std::string& getName() const;
	// const std::vector<std::string>& getArguments() const;
	const std::vector<Node>& getArguments() const;
	const std::vector<Node>& getChildren() const;
	int getLine() const;
	int getColumn() const;
	ConfigDirective findDirective(const std::string& str) const;
};

#endif // !NODE_HPP
