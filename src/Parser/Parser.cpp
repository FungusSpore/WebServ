#include "Parser.hpp"
#include "Token.hpp"

// -------------------- CONSTRUCTORS & DESTRUCTORS --------------------
Parser::Parser(const char* filename) : _file(filename), _lex(_file) {
	if (!_file)
		throw (SystemFailure(std::string(filename) + " cannot be opened"));
	_currToken = _lex.nextToken();
}

Parser::~Parser() {}

// -------------------- PRIVATE METHODS --------------------
void Parser::advanceToken() {
	_currToken = _lex.nextToken();
}

Node Parser::parseToken() {
	if (_currToken.getType() != TOKEN_STRING) {
		std::string msg = "Expected directive name at line: " + intToString(_currToken.getLine())
			+ ", col: " + intToString(_currToken.getColumn());
		throw (ParseErrorException(msg));
	}

	Node node(_currToken.getContent(), _currToken.getLine(), _currToken.getColumn());
	advanceToken();
	while (_currToken.getType() == TOKEN_STRING) {
		Node arg_node(_currToken.getContent(), _currToken.getLine(), _currToken.getColumn());
		node.addArguments(arg_node);
		advanceToken();
	}

	if (_currToken.getType() == TOKEN_SEMICOLON) {
		Node arg_node(_currToken.getContent(), _currToken.getLine(), _currToken.getColumn());
		node.addArguments(arg_node);
		advanceToken();
		return (node);
	}

	if (_currToken.getType() == TOKEN_L_BRACE) {
		advanceToken();
		while (_currToken.getType() != TOKEN_EOF && _currToken.getType() != TOKEN_R_BRACE)
			node.addChild(parseToken());
		if (_currToken.getType() != TOKEN_R_BRACE) {
			std::string msg = "Expected '}' at line: " + intToString(_currToken.getLine())
				+ ", col: " + intToString(_currToken.getColumn());
			throw (ParseErrorException(msg));
		}
		advanceToken();
		return (node);
	}

	std::string msg = "Expected ';' or '}' at line: " + intToString(_currToken.getLine())
		+ ", col: " + intToString(_currToken.getColumn());
	throw (ParseErrorException(msg));
}

// -------------------- PUBLIC METHODS --------------------
Node Parser::parseRoot() {
	Node root("root", 0, 0);

	while (_currToken.getType() != TOKEN_EOF)
		root.addChild(parseToken());
	return (root);
}

// -------------------- CLASS EXCEPTIONS --------------------
Parser::ParseErrorException::ParseErrorException(std::string& msg) throw() : _msg("Config: " + msg) {}

const char* Parser::ParseErrorException::what() const throw() {
	return (_msg.c_str());
}

Parser::ParseErrorException::~ParseErrorException() throw() {}

// -------------------- UTILS --------------------
std::string intToString(int i) {
	std::ostringstream os;

	os << i;
	return (std::string(os.str()));
}
