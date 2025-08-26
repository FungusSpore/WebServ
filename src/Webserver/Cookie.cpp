#include "Cookie.hpp"
#include <iostream>

std::string getCookieValue() {
	const static std::string valChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string value;
	srand(time(NULL));

	for (int i = 0; i < VALUE_LEN; i++) {
		value.push_back(valChars[rand() % (sizeof(valChars) - 1)]);
	}
	std::cout << "Generated cookie value: " << value << std::endl;
	return (value);
}

Cookie::Cookie(std::vector<Cookie>& cookieVector, const std::string& content) : _key("session_id"), _content(content) {
	_value = getCookieValue();

	if (!cookieVector.empty()) {
		std::vector<Cookie>::iterator it = cookieVector.begin();
		for ( ; it != cookieVector.end(); ++it) {
			if (_value == it->getValue()) {
				std::cout << "Collision detected for cookie value: " << _value << ". Generating a new value." << std::endl;
				_value = getCookieValue();
				it = cookieVector.begin();
			}
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

bool operator<(const Cookie& lhs, const Cookie& rhs) {
	if (lhs.getKey() != rhs.getKey())
		return (lhs.getKey() < rhs.getKey());
	return (lhs.getValue() < rhs.getValue());
}
