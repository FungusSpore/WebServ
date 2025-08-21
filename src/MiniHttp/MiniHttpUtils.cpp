#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include "MiniHttpUtils.hpp"
// #include <typeinfo>


void ft_strtrim(std::string& s) {
	size_t start = s.find_first_not_of(M_WS);
	size_t end = s.find_last_not_of(M_WS);
	
	if (start == std::string::npos || end == std::string::npos) {
		s.clear();
	} else {
		s = s.substr(start, end - start + 1);
	}
}

HttpMethod ft_strToHttpMethod(const std::string& method) {
	if (method == "GET") return GET;
	else if (method == "POST") return POST;
	else if (method == "DELETE") return DELETE;
	else {
		throw std::invalid_argument("Invalid HTTP method: " + method);
	}
}

std::string getMimeType(const std::string& extension) {
    if (extension == ".7z") return "application/x-7z-compressed";
    if (extension == ".avi") return "video/x-msvideo";
    if (extension == ".bmp") return "image/bmp";
    if (extension == ".css") return "text/css";
    if (extension == ".csv") return "text/csv";
    if (extension == ".eot") return "application/vnd.ms-fontobject";
    if (extension == ".exe") return "application/x-msdownload";
    if (extension == ".flv") return "video/x-flv";
    if (extension == ".gif") return "image/gif";
    if (extension == ".gz") return "application/gzip";
    if (extension == ".htm") return "text/html";
    if (extension == ".html") return "text/html";
    if (extension == ".ico") return "image/x-icon";
    if (extension == ".jpeg") return "image/jpeg";
    if (extension == ".jpg") return "image/jpeg";
    if (extension == ".js") return "application/javascript";
    if (extension == ".json") return "application/json";
    if (extension == ".m4a") return "audio/mp4";
    if (extension == ".md") return "text/markdown";
    if (extension == ".mov") return "video/quicktime";
    if (extension == ".mp3") return "audio/mpeg";
    if (extension == ".mp4") return "video/mp4";
    if (extension == ".ogg") return "audio/ogg";
    if (extension == ".otf") return "font/otf";
    if (extension == ".pdf") return "application/pdf";
    if (extension == ".png") return "image/png";
    if (extension == ".rar") return "application/vnd.rar";
    if (extension == ".svg") return "image/svg+xml";
    if (extension == ".tar") return "application/x-tar";
    if (extension == ".ttf") return "font/ttf";
    if (extension == ".txt") return "text/plain";
    if (extension == ".wasm") return "application/wasm";
    if (extension == ".wav") return "audio/wav";
    if (extension == ".webm") return "video/webm";
    if (extension == ".webp") return "image/webp";
    if (extension == ".woff") return "font/woff";
    if (extension == ".woff2") return "font/woff2";
    if (extension == ".xml") return "application/xml";
    if (extension == ".zip") return "application/zip";
    return "application/octet-stream"; // default fallback
}

std::string getFileExtension(const std::string& filename) {
	size_t pos = filename.find_last_of('.');
	if (pos == std::string::npos || pos == filename.length() - 1) {
		return "";
	}
	return filename.substr(pos);
}

bool isMimeTypeText(const std::string& mimeType) {
	return mimeType.find("text/") == 0 || mimeType == "application/json" || mimeType == "application/javascript";
}

std::string getFileContent(const std::string& path) {
	std::ifstream inFile(path.c_str(), std::ios::binary);
	if (!inFile) {
		std::cerr << "Error opening file: " << path << std::endl;
		return "";
	}

	if (isMimeTypeText(getMimeType(getFileExtension(path)))) {
		std::ostringstream oss;
		oss << inFile.rdbuf();
		return oss.str();
	} else {
		std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
		return content;
	}
}

// template <typename T>
// std::string ft_toString(T value) {
// 	std::ostringstream oss;
// 	oss << value;
// 	if (oss.fail()) {
// 		std::string errorMsg = "Failed to convert value of type " + std::string(typeid(T).name()) + " to string";
// 		throw std::runtime_error(errorMsg);
// 	}
// 	return oss.str();
// }

std::string getStatusCodeMsg(int statusCode) {
    std::map<int, std::string> m;

    // 1xx
    m[100] = "Continue";
    m[101] = "Switching Protocols";

    // 2xx
    m[200] = "OK";
    m[201] = "Created";
    m[202] = "Accepted";
    m[203] = "Non-Authoritative Information";
    m[204] = "No Content";
    m[205] = "Reset Content";
    m[206] = "Partial Content";

    // 3xx
    m[300] = "Multiple Choices";
    m[301] = "Moved Permanently";
    m[302] = "Moved Temporarily";
    m[303] = "See Other";
    m[304] = "Not Modified";
    m[305] = "Use Proxy";
    m[307] = "Temporary Redirect";

    // 4xx
    m[400] = "Bad Request";
    m[401] = "Unauthorized";
    m[402] = "Payment Required";
    m[403] = "Forbidden";
    m[404] = "Not Found";
    m[405] = "Not Allowed";
    m[406] = "Not Acceptable";
    m[407] = "Proxy Authentication Required";
    m[408] = "Request Time-out";
    m[409] = "Conflict";
    m[410] = "Gone";
    m[411] = "Length Required";
    m[412] = "Precondition Failed";
    m[413] = "Request Entity Too Large";
    m[414] = "Request-URI Too Large";
    m[415] = "Unsupported Media Type";
    m[416] = "Requested Range Not Satisfiable";
    m[417] = "Expectation Failed";
    m[421] = "Misdirected Request";
    m[426] = "Upgrade Required";
    m[428] = "Precondition Required";
    m[429] = "Too Many Requests";
    m[431] = "Request Header Fields Too Large";

    // 5xx
    m[500] = "Internal Server Error";
    m[501] = "Not Implemented";
    m[502] = "Bad Gateway";
    m[503] = "Service Temporarily Unavailable";
    m[504] = "Gateway Time-out";
    m[505] = "HTTP Version Not Supported";

	if (m.find(statusCode) == m.end()) {
		return "Unknown Status Code";
	} else if (statusCode < 100 || statusCode >= 600) {
		return "Invalid Status Code";
	} else {
		return m[statusCode];
	}
}

std::string getCurrentTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tstruct);
	return std::string(buf);
}


bool pathExists(const std::string& p) {
	struct stat st;
	return ::stat(p.c_str(), &st) == 0;
}

bool isDirectory(const std::string& p) {
	struct stat st;
	return ::stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool isFile(const std::string& p) {
	struct stat st;
	return ::stat(p.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool isSymlink(const std::string& p) {
	struct stat st;
	return ::stat(p.c_str(), &st) == 0 && S_ISLNK(st.st_mode);
}

std::string joinPath(const std::string& a, const std::string& b) {
	if (a.empty())
		return b;
	if (b.empty())
		return a;
	if (a[a.size()-1] == '/' && b[0] == '/')
		return a + b.substr(1);
	if (a[a.size()-1] != '/' && b[0] != '/')
		return a + "/" + b;
	return a + b;
}

std::string normalizeUnderRoot(const std::string& root, const std::string& path) {
	// Normalize the path under the given root directory with handling of '.', '..', and empty components.
	
	// std::cout << "Normalizing path: " << path << " under root: " << root << std::endl;

	std::vector<std::string> components;
	std::istringstream iss(path);
	std::string part;
	while (std::getline(iss, part, '/')) {
		if (!part.empty() && part != ".") {
			if (part == "..") {
				if (!components.empty())
					components.pop_back();
			} else {
				components.push_back(part);
			}
		}
	}

	std::string normalizedPath = root;
	for (std::vector<std::string>::const_iterator it = components.begin(); it != components.end(); ++it) {
		normalizedPath = joinPath(normalizedPath, *it);
	}

	return normalizedPath;
}

std::string getDefaultIndexFile(const std::string& path) {
	const char* defaultIndexFiles[] = {
		"index.html",
		"index.htm",
		"index.php",
		"index.shtml",
		"index.txt"
	};

	const std::size_t size = sizeof(defaultIndexFiles) / sizeof(defaultIndexFiles[0]);

	for (std::size_t i = 0; i < size; ++i) {
		std::string indexFilePath = joinPath(path, defaultIndexFiles[i]);
		if (isFile(indexFilePath)) {
			return indexFilePath;
		}
	}
	return "";
}

bool isSlashRedirect(const std::string& path) {
	// Check if the path ends with a slash and is not just a single slash
	return !path.empty() && path[path.size() - 1] != '/';
}
