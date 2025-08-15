#include "Lexer.hpp"

// -------------------- CONSTRUCTORS & DESTRUCTORS --------------------
Lexer::Lexer(std::ifstream& file) : _file(file), _line(1), _col(0), _currChar(' ') {}

Lexer::Lexer(const Lexer& other) : _file(other._file), _line(other._line),
	_col(other._col), _currChar(other._currChar) {}

Lexer::~Lexer() {}

// -------------------- PRIVATE METHODS --------------------
void Lexer::getChar() {
	_currChar = _file.get();
	if (_currChar == '\n') {
		_line++;
		_col = 0;
	}
	else
		_col++;
}

void Lexer::skipWhitespaceAndComments() {
	while (_currChar != EOF) {
		if (std::isspace(_currChar)) {
			getChar();
			continue;
		}
		if (_currChar == '#') {
			while (_currChar != '\n' && _currChar != EOF)
				getChar();
			continue;
		}
		break ;
	}
}

// -------------------- PUBLIC METHODS --------------------
Token Lexer::nextToken() {
	skipWhitespaceAndComments();

	if(_currChar == EOF)
		return (Token(TOKEN_EOF, "", _line, _col));

	if (_currChar == '\'' || _currChar == '"') {
		char quoteType = _currChar;
		std::string temp;
		int startLine = _line;
		int startCol = _col;

		getChar();
		while (_currChar != EOF && _currChar != quoteType) {
			temp.push_back(_currChar);
			getChar();
		}
		if (_currChar == quoteType)
			getChar();
		return (Token(TOKEN_STRING, temp, startLine, startCol));
	}
	
	if (_currChar == '{') {
		int startLine = _line;
		int startCol = _col;
		getChar();
		return (Token(TOKEN_L_BRACE, "{", startLine, startCol));
	}
	if (_currChar == '}') {
		int startLine = _line;
		int startCol = _col;
		getChar();
		return (Token(TOKEN_R_BRACE, "}", startLine, startCol));
	}
	if (_currChar == ';') {
		int startLine = _line;
		int startCol = _col;
		getChar();
		return (Token(TOKEN_SEMICOLON, ";", startLine, startCol));
	}

	std::string temp;
	int startLine = _line;
	int startCol = _col;

	while (_currChar != EOF && !std::isspace(_currChar) && _currChar != '{'
		&& _currChar != '}' && _currChar != ';' && _currChar != '#') {
		temp.push_back(static_cast<char>(_currChar));
		getChar();
	}
	return (Token(TOKEN_STRING, temp, startLine, startCol));
}
