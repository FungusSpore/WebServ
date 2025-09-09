#include <iostream>
#include "MiniHttp.hpp"
#include "CGI.hpp"
#include "Cookie.hpp"
#include "Socket.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include "MiniHttpUtils.hpp"
#include "WebServer.hpp"

MiniHttp::MiniHttp(Socket& socket, WebServer& server) : _socket(socket), _server(server), _request(socket) {}

MiniHttp::~MiniHttp() {}

// ===================================================================
// CGI HELPER
// ===================================================================

std::string MiniHttp::getCgiHeader(std::vector<char>& cgiBuffer) {
	std::string header;
	std::string::size_type s = ft_vectorFind(cgiBuffer, M_CRLF2);
	if (s == std::string::npos) {
		// Try with just \n\n for Unix-style line endings just safety
		s = ft_vectorFind(cgiBuffer, "\n\n");
		if (s == std::string::npos)
			return header;
		header = std::string(cgiBuffer.begin(), cgiBuffer.begin() + s + 2);
		cgiBuffer.erase(cgiBuffer.begin(), cgiBuffer.begin() + s + 2);
	} else {
		header = std::string(cgiBuffer.begin(), cgiBuffer.begin() + s + 4);
		cgiBuffer.erase(cgiBuffer.begin(), cgiBuffer.begin() + s + 4);
	}
	return header;
}

// ===================================================================
// COOKIE HANDLING
// ===================================================================

void MiniHttp::parseCgiCookie(std::string& cgiHeaders) {
	Cookie* cookie = NULL;

	// find cgi_cookie header if exists
	size_t cookiePos = cgiHeaders.find("cgi_cookie:");
	if (cookiePos != std::string::npos) {
		size_t lineEnd = cgiHeaders.find("\r\n", cookiePos);
		size_t lineEndLength = 2;
		if (lineEnd == std::string::npos) {
			lineEnd = cgiHeaders.find("\n", cookiePos);
			lineEndLength = 1;
		}
		if (lineEnd != std::string::npos) {
			std::string cookieLine = cgiHeaders.substr(cookiePos + 11, lineEnd - (cookiePos + 11));
			ft_strtrim(cookieLine);

			if (cookieLine.empty()) {
				cgiHeaders.erase(cookiePos, lineEnd - cookiePos + lineEndLength);
				return;
			}


			std::istringstream iss(cookieLine);
			std::string token;
			std::vector<std::string> cookies;
			while (std::getline(iss, token, ';')) {
				cookies.push_back(token);
			}

			std::vector<std::pair<std::string, std::string> > cookieMap;

			for (std::vector<std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it) {
				ft_strtrim(*it);
				size_t pos = (*it).find('=');
				if (pos != std::string::npos) {
					std::string key = (*it).substr(0, pos);
					std::string value = (*it).substr(pos + 1);
					ft_strtrim(key);
					ft_strtrim(value);

					cookieMap.push_back(std::make_pair(key, value));
				}
			}

			// it might have multiple session_id cookies, take the first valid one
			for (std::vector<std::pair<std::string, std::string> >::iterator it = cookieMap.begin(); it != cookieMap.end(); ++it) {
				if (it->first == "session_id") {
					if (it->second.find("DESTROY:") == 0) {
						// remove DESTROY: prefix
						std::string oldSessionId = it->second.substr(8);
						std::string expireCookieHeader = "Set-Cookie: session_id=" + oldSessionId + "; Path=/; HttpOnly; Max-Age=0\r\n";
						
						size_t insertPos = (lineEnd != std::string::npos) ? lineEnd + 2 : cgiHeaders.length();
						cgiHeaders.insert(insertPos, expireCookieHeader);
						
						_server.deleteCookie(oldSessionId);
						cookie = _server.addCookie("Guest");

						std::string newCookieHeader = "Set-Cookie: session_id=" + cookie->getValue() + "; Path=/; HttpOnly\r\n";
						cgiHeaders.insert(insertPos + expireCookieHeader.length(), newCookieHeader);
						
						break;
					} else {
						cookie = _server.matchCookieValue(it->second);
						if (cookie) {
							break;
						}
					}
				}
			}

			if (cookie) {
				for (std::vector<std::pair<std::string, std::string> >::iterator it = cookieMap.begin(); it != cookieMap.end(); ++it) {
					if (it->first == "user_id") {
						if (!it->second.empty() && it->second != "Guest") {
							cookie->setContent(it->second);
							break;
						}
					}
				}
			}


		}
		cgiHeaders.erase(cookiePos, lineEnd - cookiePos + lineEndLength);
	}
}

// ===================================================================
// MAIN PROCESSING METHOD
// ===================================================================

bool MiniHttp::run() {
	if (!_request.parseRequest()) {
		return false;
	}

	MiniHttpResponse response(_server, _request, _socket);
	response.parseResponse();

	_socket.write_buffer = response.buildResponse();

	_request.clearRequest();

	return true;
}

bool MiniHttp::validateCGI() {
	if (_socket.read_buffer.empty()) {
		return false;
	}
	
	try {
		std::vector<char> cgiBuffer = _socket.read_buffer;
		std::string cgiHeaders = getCgiHeader(cgiBuffer);

		if (cgiHeaders.empty()) {
			return false;
		}

		parseCgiCookie(cgiHeaders);

		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		
		if (!cgiHeaders.empty()) {
			size_t headerEndPos = cgiHeaders.find("\r\n\r\n");
			if (headerEndPos != std::string::npos) {
				cgiHeaders = cgiHeaders.substr(0, headerEndPos);
			} else {
				headerEndPos = cgiHeaders.find("\n\n");
				if (headerEndPos != std::string::npos) {
					cgiHeaders = cgiHeaders.substr(0, headerEndPos);
				}
			}
			response << cgiHeaders << "\r\n";
		}
		
		if (cgiHeaders.find("Content-Length:") == std::string::npos && 
			cgiHeaders.find("content-length:") == std::string::npos) {
			response << "Content-Length: " << cgiBuffer.size() << "\r\n";
		}

		response << "\r\n";
		
		std::string headers = response.str();
		
		// Handle binary data properly not convert to string
		_socket.toSend->write_buffer.assign(headers.begin(), headers.end());
		_socket.toSend->write_buffer.insert(_socket.toSend->write_buffer.end(), 
											cgiBuffer.begin(), cgiBuffer.end());
		_socket.read_buffer.clear();
		
		return true;
		
	} catch (const std::exception& e) {
		std::cerr << "Error processing CGI output: " << e.what() << std::endl;
		// dont exit just return eerror code as fallbakk
		std::string errStr = "HTTP/1.1 500 Internal Server Error\r\n"
					   "Content-Type: text/html\r\n"
					   "Content-Length: 50\r\n"
					   "Connection: close\r\n"
					   "\r\n"
					   "<html><body><h1>500 Internal Server Error</h1></body></html>";
		_socket.write_buffer.assign(errStr.begin(), errStr.end());
		_socket.read_buffer.clear();
		return true;
	}
}
