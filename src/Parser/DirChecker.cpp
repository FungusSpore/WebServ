#include "DirChecker.hpp"
#include "Location.hpp"
#include "Parser.hpp"

// --------------- UTILS ---------------
static bool validDigitString(const std::string& string) {
	if (string.empty())
		return (false);
	for (std::string::size_type i = 0; i < string.size(); i++) {
		if (!std::isdigit(static_cast<char>(string[i])))
			return (false);
	}
	return (true);
}

static bool validPort(const std::string& string) {
	int port = std::atoi(string.c_str());
	if (port < 1 || port > 65535 || !validDigitString(string))
		return (false);
	return (true);
}

static bool validIp(const std::string& string) {
	int ip1, ip2, ip3, ip4;
	char dot1, dot2, dot3;

	std::istringstream iss(string);
	iss >> ip1 >> dot1 >> ip2 >> dot2 >> ip3 >> dot3 >> ip4;

	if (ip1 < 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 || ip3 < 0 || ip3 > 255
		|| ip4 < 0 || ip3 > 255 || dot1 != '.' || dot2 != '.' || dot3 != '.')
		return (false);
	if (!iss.eof())
		return (false);
	return (true);
}

static bool validIpPort(const std::string& string) {
	std::string::size_type pos = string.find(":");
	if (pos == std::string::npos) {
		if (!validPort(string))
			return (false);
	}
	else {
		std::string ipString = string.substr(0, pos);
		std::string portString = string.substr(pos + 1);
		if (!validIp(ipString) || !validPort(portString))
			return (false);
	}
	return (true);
}

// --------------- PUBLIC METHODS ---------------
static void checkEmptyArgument(const Node& node, const std::vector<Node>& arg, size_t argSize, bool reqContextBlock) {
	if (argSize == 1 && !reqContextBlock) {
		std::string msg = node.getName() + " argument not specified at line: " +
			intToString(arg[0].getLine()) + " col: " + intToString(arg[0].getColumn());
		throw (Parser::ParseErrorException(msg));
	}
}

static void checkExpectedArgumentSize(const Node& node, const std::vector<Node>& arg, size_t argSize, size_t expectedArgSize) {
	if (argSize > expectedArgSize + 1) {
		std::string msg = "extra " + node.getName() + " argument specified at line: " +
			intToString(arg[1].getLine()) + " col: " + intToString(arg[1].getColumn());
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkArguments(const Node& node, int expectedArgSize, bool reqContextBlock) {
	const std::vector<Node>& arg = node.getArguments();
	size_t argSize = arg.size();

	checkEmptyArgument(node, arg, argSize, reqContextBlock);
	if (expectedArgSize != -1)
		checkExpectedArgumentSize(node, arg, argSize, static_cast<size_t>(expectedArgSize));
}

void DirChecker::checkContextBlock(const Node& node) {
	const std::vector<Node>& arg = node.getArguments();
	if (arg.back().getName() != ";") {
		std::string msg = node.getName() + " directive at line: " +
			intToString(node.getLine()) + " col: " +
			intToString(node.getColumn()) + " is invalid. (invalid context block)";
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkDuplicateDirective(const Node& node, bool isSet) {
	if (isSet) {
		std::string msg = "duplicate " + node.getName() + " at line: " +
			intToString(node.getLine()) + " col: " + intToString(node.getColumn());
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkRootDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);
}

void DirChecker::checkAliasDirective(const Node &node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);
}

void DirChecker::checkServerNameDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);
}

void DirChecker::checkIndexDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, -1, false);
}

void DirChecker::checkAllowedMethodDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, -1, false);

	const std::vector<Node>& arg = node.getArguments();
	int getCount = 0;
	int postCount = 0;
	int deleteCount = 0;
	int headCount = 0;
	std::vector<Node>::const_iterator it = arg.begin();
	for ( ; it != arg.end() - 1; ++it) {
		if (it->getName() == "GET")
			getCount++;
		else if (it->getName() == "POST")
			postCount++;
		else if (it->getName() == "DELETE")
			deleteCount++;
		else if (it->getName() == "HEAD")
			headCount++;
		else {
			std::string msg = "unexpected " + it->getName() + " at line: " +
				intToString(it->getLine()) + " col: " + intToString(it->getColumn()) +
				" in " + node.getName() + " directive";
			throw (Parser::ParseErrorException(msg));
		}
		if (getCount > 1 || postCount > 1 || deleteCount > 1 || headCount > 1) {
			std::string msg = "duplicate " + it->getName() + " method at line: " +
				intToString(it->getLine()) + " col: " + intToString(it->getColumn());
			throw (Parser::ParseErrorException(msg));
		}
	}
}

// supported status code for error page: 300 to 599
void DirChecker::checkErrorPageDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, -1, false);

	const std::vector<Node>& arg = node.getArguments();
	size_t argSize = arg.size();

	for (size_t i = 0; i < argSize - 2; i++) {
		int statusCode = std::atoi(arg[i].getName().c_str());
		if (statusCode < 300 || statusCode > 599 || !validDigitString(arg[i].getName())) {
			std::string msg = "invalid value " + arg[i].getName() + " at line:" +
				intToString(arg[i].getLine()) + " col: " +
				intToString(arg[i].getColumn()) + " in " + node.getName() +
				" directive";
			throw (Parser::ParseErrorException(msg));
		}
	}
}

void DirChecker::checkClientMaxBodySizeDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);

	const std::vector<Node>& arg = node.getArguments();
	if (!validDigitString(arg[0].getName())) {
		std::string msg = "invalid value " + arg[1].getName() + " at line:" +
			intToString(arg[0].getLine()) + " col: " +
			intToString(arg[0].getColumn()) + " in " + node.getName() +
			" directive";
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkClientTimeoutDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);

	const std::vector<Node>&arg = node.getArguments();
	if (!validDigitString(arg[0].getName())) {
		std::string msg = "invalid value " + arg[1].getName() + " at line:" +
			intToString(arg[0].getLine()) + " col: " +
			intToString(arg[0].getColumn()) + " in " + node.getName() +
			" directive";
		throw (Parser::ParseErrorException(msg));
	}
}

// min port 1, max port 65535
void DirChecker::checkListenDirective(const Node& node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);

	const std::vector<Node>& arg = node.getArguments();
	if (!validIpPort(arg[0].getName())) {
		std::string msg = "invalid listen argument " + arg[0].getName() + " at line: " +
			intToString(arg[0].getLine()) + " col: " + intToString(arg[0].getColumn());
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkLocationDirective(const Node& node, const std::set<std::string>& locationTargets) {
	DirChecker::checkArguments(node, 1, true);
	const std::vector<Node>& arg = node.getArguments();
	const std::string& target = arg[0].getName();

	if (locationTargets.find(target) != locationTargets.end()) {
		std::string msg = "duplicate location " + target + " at line: " +
			intToString(arg[0].getLine()) + " col: " + intToString(arg[0].getColumn());
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkLocationModeDirective(const Node &node, LocationMode mode) {
	if (mode != DEFAULT_LOCATIONMODE) {
		std::string msg = "invalid " + node.getName() + " at line: " + 
			intToString(node.getLine()) + " col: " + intToString(node.getColumn()) +
			". (Location Mode already assign for this location block)";
		throw(Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkPathModeDirective(const Node &node, PathMode mode) {
	if (mode != DEFAULT_PATHMODE) {
		std::string msg = "invalid " + node.getName() + " at line: " + 
			intToString(node.getLine()) + " col: " + intToString(node.getColumn()) +
			". (Path Mode already assign for this location block)";
		throw(Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkAutoindexDirective(const Node &node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);

	const std::vector<Node>& arg = node.getArguments();
	if (arg[0].getName() != "on" && arg[0].getName() != "off") {
		std::string msg = "invalid value " + arg[0].getName() + " at line: " +
			intToString(arg[0].getLine()) + " col: " +
			intToString(arg[0].getColumn()) + " in " + node.getName() +
			" directive";
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkCgiDirective(const Node &node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);

	const std::vector<Node>& arg = node.getArguments();
	if (arg[0].getName() != "on" && arg[0].getName() != "off") {
		std::string msg = "invalid value " + arg[0].getName() + " at line: " +
			intToString(arg[0].getLine()) + " col: " +
			intToString(arg[0].getColumn()) + " in " + node.getName() +
			" directive";
		throw (Parser::ParseErrorException(msg));
	}
}

void DirChecker::checkProxyPassDirective(const Node &node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 1, false);
}

void DirChecker::checkRedirectionDirective(const Node &node) {
	DirChecker::checkContextBlock(node);
	DirChecker::checkArguments(node, 2, false);
}
