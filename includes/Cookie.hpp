#ifndef COOKIE_HPP
#define COOKIE_HPP

#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

#define VALUE_LEN 16
// key = session_id
// value = 16char alphanumeric (with caps) string
// content = std::content string
class Cookie {
private:
	std::string _key; // session_id
	std::string _value; // unique token
	std::string _content; // userName

public:
	Cookie(std::vector<Cookie>& cookieSet, const std::string& content);
	~Cookie();

	const std::string& getKey() const;
	const std::string& getValue() const;
	const std::string& getContent() const;

	void setKey(const std::string& key);
	void setValue(const std::string& value);
	void setContent(const std::string& content);
};

bool operator<(const Cookie& lhs, const Cookie& rhs);

#endif // !COOKIE_HPP
