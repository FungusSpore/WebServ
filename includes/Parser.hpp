#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <set>
#include <fstream>
#include <sstream>
#include "Exceptions.hpp"
#include "Lexer.hpp"
#include "Node.hpp"

class Parser {
private:
	std::ifstream _file;
	Lexer _lex;
	Token _currToken;

	void advanceToken();
	Node parseToken();

public:
	Parser(const char* filename);
	~Parser();

	class ParseErrorException : public std::exception {
	private:
		std::string _msg;

	public:
		ParseErrorException(std::string& msg) throw();
		~ParseErrorException() throw();
		const char* what() const throw();
	};

	Node parseRoot();
};

std::string intToString(int i);

#endif // !PARSER_HPP
