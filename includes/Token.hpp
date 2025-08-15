#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TokenType {
	// TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_L_BRACE,
	TOKEN_R_BRACE,
	TOKEN_SEMICOLON,
	TOKEN_EOF,
	TOKEN_DEFAULT
};

class Token {
private:
	TokenType _type;
	std::string _content;
	int _line;
	int _col;

public:
	Token();
	Token(TokenType type, const std::string& content, int line, int col);
	// Token(const std::string& content, int line, int col);
	Token(const Token& other);
	Token& operator=(const Token& other);
	~Token();

	TokenType getType() const;
	const std::string& getContent() const;
	int getLine() const;
	int getColumn() const;
};

#endif // !TOKEN_HPP
