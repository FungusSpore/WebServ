#ifndef LEXER_HPP
#define LEXER_HPP

#include <fstream>
#include <cctype>
#include "Token.hpp"

class Lexer {
private:
	std::ifstream& _file;
	int _line;
	int _col;
	int _currChar;

	void getChar();
	void skipWhitespaceAndComments();

public:
	Lexer(std::ifstream& file);
	Lexer(const Lexer& other);
	~Lexer();

	Token nextToken();
};

#endif // !LEXER_HPP
