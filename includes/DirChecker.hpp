#ifndef DIRCHECKER_HPP
#define DIRCHECKER_HPP

#include <cstdlib>
#include "Parser.hpp"
#include "Location.hpp"

class DirChecker {
private:
	DirChecker();
	~DirChecker();

public:
	static void checkArguments(const Node& node, int expectedArgSize, bool reqContextBlock);
	static void checkContextBlock(const Node& node);
	static void checkDuplicateDirective(const Node& node, bool isSet);
	static void checkRootDirective(const Node& node);
	static void checkAliasDirective(const Node& node);
	static void checkServerNameDirective(const Node& node);
	static void checkIndexDirective(const Node& node);
	static void checkAllowedMethodDirective(const Node& node);
	static void checkErrorPageDirective(const Node& node);
	static void checkClientMaxBodySizeDirective(const Node& node);
	static void checkClientTimeoutDirective(const Node& node);
	static void checkListenDirective(const Node& node);
	static void checkLocationDirective(const Node& node, const std::set<std::string>& locationTargets);
	static void checkLocationModeDirective(const Node& node, LocationMode mode);
	static void checkPathModeDirective(const Node& node, PathMode mode);
	static void checkAutoindexDirective(const Node& node);
	static void checkCgiDirective(const Node& node);
	static void checkProxyPassDirective(const Node& node);
	static void checkRedirectionDirective(const Node& node);
};

#endif // !DIRCHECKER_HPP
