#include "Token.hpp"

// -------------------- CONSTRUCTORS & DESTRUCTORS --------------------
Token::Token() : _type(TOKEN_DEFAULT), _line(0), _col(0) {}

Token::Token(TokenType type, const std::string& content, int line, int col) :
	_type(type), _content(content), _line(line), _col(col) {}

// Token::Token(const std::string& content, int line, int col) :
// 	_type(TOKEN_DEFAULT), _content(content), _line(line), _col(col) {}

Token::Token(const Token& other) : _type(other._type), _content(other._content),
	_line(other._line), _col(other._col) {}

Token& Token::operator=(const Token& other) {
	if (this != &other) {
		this->_type = other._type;
		this->_content = other._content;
		this->_line = other._line;
		this->_col = other._col;
	}
	return (*this);
}

Token::~Token() {}

// -------------------- PUBLIC METHODS --------------------
TokenType Token::getType() const {
	return (_type);
}

const std::string& Token::getContent() const {
	return (_content);
}

int Token::getLine() const {
	return (_line);
}

int Token::getColumn() const {
	return (_col);
}
