#include "Cookie.hpp"

std::string getCookieValue() {
	const static std::string valChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string value;
	srand(time(NULL));

	for (int i = 0; i < VALUE_LEN; i++) {
		value.push_back(valChars[rand() % (sizeof(valChars) - 1)]);
	}
	return (value);
}

Cookie::Cookie(const std::set<Cookie>& cookieSet, const std::string& content) : _key("session_id"), _content(content) {
	std::string value = getCookieValue();

	std::set<Cookie>::iterator it = cookieSet.begin();
	for ( ; it != cookieSet.end(); ++it) {
		if (value == it->getValue()) {
			value = getCookieValue();
			it = cookieSet.begin();
		}
	}
}

Cookie::~Cookie() {}

const std::string& Cookie::getKey() const {
	return (_key);
}

const std::string& Cookie::getValue() const {
	return (_value);
}

const std::string& Cookie::getContent() const {
	return (_content);
}

void Cookie::setKey(const std::string& key) {
	_key = key;
}

void Cookie::setValue(const std::string& value) {
	_value = value;
}

void Cookie::setContent(const std::string& content) {
	_content = content;
}
