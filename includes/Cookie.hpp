#ifndef COOKIE_HPP
#define COOKIE_HPP

#include <string>
#include <set>
#include <cstdlib>
#include <ctime>

#define VALUE_LEN 16
// key = session_id
// value = 16char alphanumeric (with caps) string
// content = std::content string
class Cookie {
private:
	std::string _key;
	std::string _value;
	std::string _content;

public:
	Cookie(const std::set<Cookie>& cookieSet, const std::string& content);
	~Cookie();

	const std::string& getKey() const;
	const std::string& getValue() const;
	const std::string& getContent() const;

	void setKey(const std::string& key);
	void setValue(const std::string& value);
	void setContent(const std::string& content);
};

#endif // !COOKIE_HPP
