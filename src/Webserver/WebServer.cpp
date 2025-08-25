#include "WebServer.hpp"

// void printNode(const Node& node, int indent = 0) {
//     std::string prefix(indent * 2, ' ');
//
//     // Print the node name and args
//     std::cout << prefix << node.getName();
//     for (const auto& arg : node.getArguments()) {
//         std::cout << " " << arg.getName();
//     }
//
//     // Print position (optional)
//     std::cout << "    [line " << node.getLine() << ", col " << node.getColumn() << "]";
//
//     // If it has children, open a block
//     if (!node.getChildren().empty()) {
//         std::cout << " {\n";
//         for (const auto& child : node.getChildren()) {
//             printNode(child, indent + 1);
//         }
//         std::cout << prefix << "}";
//     }
//
//     std::cout << "\n";
// }

// --------------- WEBSERVER ----------------
void WebServer::checkTopLevelBlock(const Node& topLevelNode) {
	size_t childrenSize = topLevelNode.getChildren().size();
	if (childrenSize == 0) {
		std::string msg = "config file is empty";
		throw (Parser::ParseErrorException(msg));
	}
	if (childrenSize > 1 || topLevelNode.getChildren()[0].getName() != std::string("http")) {
		std::string msg = "config file only accept one http block";
		throw (Parser::ParseErrorException(msg));
	}
}

void WebServer::checkHttpBlock(const Node& httpNode) {
	if (httpNode.getArguments().size() != 0) {
		const Node& arg = httpNode.getArguments()[0];
		std::string msg = "unexpected " + arg.getName() + " at line: " +
			intToString(arg.getLine()) + " col: " + intToString(arg.getColumn());
		throw (Parser::ParseErrorException(msg));
	}
	if (httpNode.getChildren().size() < 1) {
		std::string msg = "http block at line: " + intToString(httpNode.getLine()) +
			" col: " + intToString(httpNode.getColumn()) + " is empty";
		throw (Parser::ParseErrorException(msg));
	}
}

WebServer::WebServer(const char* filename) {
	Parser parser(filename);
	Node root = parser.parseRoot();
	// printNode(root);
	
	checkTopLevelBlock(root);

	const Node& http = root.getChildren()[0];
	checkHttpBlock(http);

	// init Server
	const std::vector<Node>& servers = http.getChildren();
	std::vector<Node>::const_iterator it_servers = servers.begin();
	for ( ; it_servers != servers.end(); ++it_servers) {
		Server serv(*it_servers);
		ServerKey serv_key(serv.getIp(), serv.getPort(), serv.getServerName());
		std::vector<ServerKey>::const_iterator it = _serverKeys.begin();
		for ( ; it != _serverKeys.end(); ++it) {
			if (serv_key._port == it->_port && serv_key._serverName == it->_serverName) {
				std::string msg = "duplicate port " + serv_key._port
					+ " and server_name " + serv_key._serverName;
				throw (Parser::ParseErrorException(msg));
			}
		}
		_ports.push_back(serv_key._port);
		_serverKeys.push_back(serv_key);
		_serverMap.insert(std::pair<ServerKey, Server>(serv_key, serv));
	}
}

WebServer::WebServer(const WebServer& other) : _ports(other._ports), _serverMap(other._serverMap) {}

WebServer& WebServer::operator=(const WebServer& other) {
	if (this != &other) {
		this->_ports = other._ports;
		this->_serverMap = other._serverMap;
	}
	return (*this);
}

WebServer::~WebServer() {}

// if serverName empty : match 2 ? match 3 (not implement yet)
const Server* WebServer::matchServer(const ServerKey& key) const {
	// std::map<ServerKey, Server>::const_iterator it = _serverMap.begin();
	//
	// if (key._serverName.empty()) {
	// 	for ( ; it != _serverMap.end(); ++it) {
	// 		if (key._ip == it->first._ip && key._port == it->first._port)
	// 			return (&it->second);
	// 	}
	// }
	// else {
	// 	for ( ; it != _serverMap.end(); ++it) {
	// 		if (key._ip == it->first._ip && key._port == it->first._port
	// 			&& key._serverName == it->first._serverName)
	// 			return (&it->second);
	// 	}
	// }
	
	// std::cout << "Available servers:\n";
	// for (std::map<ServerKey, Server>::const_iterator pair = _serverMap.begin();
	//      pair != _serverMap.end(); ++pair) {
		// std::cout << "IP: " << pair->first._ip
		//           << ", Port: " << pair->first._port
		//           << ", ServerName: " << pair->first._serverName << "\n";
	// }
	std::map<ServerKey, Server>::const_iterator it = _serverMap.find(key);

	if (it != _serverMap.end())
		return (&it->second);
	return (NULL);
}

const std::vector<std::string>& WebServer::getPorts() const {
	return (_ports);
}

long WebServer::getClientTimeout(const std::string& port) const {
	std::map<ServerKey, Server>::const_iterator it = _serverMap.begin();

	for (; it != _serverMap.end(); it++)
		if (it->first._port == port)
			return it->second.getClientTimeout();
	return (-1);
}
