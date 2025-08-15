#include "Node.hpp"

// -------------------- PRIVATE STATIC --------------------
const std::map<std::string, ConfigDirective> Node::_directiveMap = initDirectiveMap();

std::map<std::string, ConfigDirective> Node::initDirectiveMap() {
	std::map<std::string, ConfigDirective> dirMap;

	dirMap.insert(std::pair<std::string, ConfigDirective>("http", DIR_HTTP));
	dirMap.insert(std::pair<std::string, ConfigDirective>("server", DIR_SERVER));
	dirMap.insert(std::pair<std::string, ConfigDirective>("location", DIR_LOCATION));
	dirMap.insert(std::pair<std::string, ConfigDirective>("listen", DIR_LISTEN));
	dirMap.insert(std::pair<std::string, ConfigDirective>("server_name", DIR_SERVER_NAME));
	dirMap.insert(std::pair<std::string, ConfigDirective>("root", DIR_ROOT));
	dirMap.insert(std::pair<std::string, ConfigDirective>("alias", DIR_ALIAS));
	dirMap.insert(std::pair<std::string, ConfigDirective>("error_page", DIR_ERROR_PAGE));
	dirMap.insert(std::pair<std::string, ConfigDirective>("allowed_method", DIR_ALLOWED_METHOD));
	dirMap.insert(std::pair<std::string, ConfigDirective>("index", DIR_INDEX));
	dirMap.insert(std::pair<std::string, ConfigDirective>("cgi", DIR_CGI));
	dirMap.insert(std::pair<std::string, ConfigDirective>("autoindex", DIR_AUTOINDEX));
	dirMap.insert(std::pair<std::string, ConfigDirective>("proxy_pass", DIR_PROXY_PASS));
	dirMap.insert(std::pair<std::string, ConfigDirective>("redirection", DIR_REDIRECTION));
	dirMap.insert(std::pair<std::string, ConfigDirective>("client_max_body_size", DIR_CLIENT_MAX_BODY_SIZE));
	dirMap.insert(std::pair<std::string, ConfigDirective>("client_timeout", DIR_CLIENT_TIMEOUT));
	return (dirMap);
}

// -------------------- CONSTRUCTORS & DESTRUCTORS --------------------
Node::Node() : _line(0), _col(0) {}

Node::Node(const std::string& name, int line, int col) : _name(name),
	_line(line), _col(col) {}

Node::Node(const Node& other) : _name(other._name), _args(other._args),
	_children(other._children), _line(other._line), _col(other._col) {}

Node& Node::operator=(const Node& other) {
	if (this != &other) {
		this->_name = other._name;
		this->_args = other._args;
		this->_children = other._children;
		this->_line = other._line;
		this->_col = other._col;
	}
	return (*this);
}

Node::~Node() {}

// -------------------- PUBLIC METHODS --------------------
void Node::setName(const std::string& name) {
	_name = name;
}

void Node::setPosition(int line, int col) {
	_line = line;
	_col = col;
}

// void Node::addArguments(const std::string& arg) {
// 	_args.push_back(arg);
// }

void Node::addArguments(const Node& arg) {
	_args.push_back(arg);
}

void Node::addChild(const Node& children) {
	_children.push_back(children);
}

const std::string& Node::getName() const {
	return (_name);
}

// const std::vector<std::string>& Node::getArguments() const {
// 	return (_args);
// }

const std::vector<Node>& Node::getArguments() const {
	return (_args);
}

const std::vector<Node>& Node::getChildren() const {
	return (_children);
}

int Node::getLine() const {
	return (_line);
}

int Node::getColumn() const {
	return (_col);
}

ConfigDirective Node::findDirective(const std::string& str) const {
	std::map<std::string, ConfigDirective>::const_iterator it = _directiveMap.find(str);
	if (it != _directiveMap.end())
		return (it->second);
	return (DIR_UNKNOWN);
}
