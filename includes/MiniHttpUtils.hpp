#ifndef MINIHTTPUTILS_HPP
#define MINIHTTPUTILS_HPP

#include <iostream>
#include "WebServer.hpp"
#include <typeinfo>
#include <vector>

#define M_WS " \t\n\r\f\v"
#define M_CRLF "\r\n"
#define M_CRLF2 "\r\n\r\n"

void ft_strtrim(std::string& s);
HttpMethod ft_strToHttpMethod(const std::string& method);
std::string getMimeType(const std::string& extension);
std::string getFileExtension(const std::string& filename);
bool isMimeTypeText(const std::string& mimeType);
std::vector<char> getFileContent(const std::string& path);
std::string getStatusCodeMsg(int statusCode);
std::string getCurrentTime();

bool pathExists(const std::string& p);
bool isDirectory(const std::string& p);
bool isFile(const std::string& p);
bool isSymlink(const std::string& p);
bool isPathandDirectory(const std::string& p);
std::string joinPath(const std::string& a, const std::string& b);
std::string normalizeUnderRoot(const std::string& path, const std::string& root);
std::string getDefaultIndexFile(const std::string& path);
bool isSlashRedirect(const std::string& path);

std::size_t ft_vectorFind(const std::vector<char>& vec, const std::string& find);
std::vector<char> ft_vectorSubstr(const std::vector<char>& vec, std::size_t pos, std::size_t len);

template <typename T>
std::string ft_toString(T value) {
	std::ostringstream oss;
	oss << value;
	if (oss.fail()) {
		std::string errorMsg = "Failed to convert value of type " + std::string(typeid(T).name()) + " to string";
		throw std::runtime_error(errorMsg);
	}
	return oss.str();
}

#endif
