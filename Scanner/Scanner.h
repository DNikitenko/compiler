#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <iostream>
#include "Errors.h"

const char chNewLine = 10;

const string Reserved[] = {"asm", "auto", "break", "case", "catch", "class","continue", "default", "delete", "do",
						  "double", "else", "enum", "extern", "for", "friend", "goto", "if", "inline", "long", "new", "operator",
						  "private", "protected", "public", "register", "return", "short", "signed", "sizeof", "static", 
						  "struct", "switch", "template", "this", "throw", "try", "typedef", "typeid", "union", "unsigned", "virtual",
						  "volatile", "while"};

const string Type[]     = {"char", "float", "int", "string", "void"};

enum TokenType			  {ttChar = 0, ttDelimiter, ttEOF, ttIdentifier, ttInt, ttOperation,
						   ttReal, ttReserved, ttString, ttPlus, ttMinus, ttMul, ttDiv, ttOpenBracket, 
						   ttClosingBracket, ttLShift, ttRShift, ttLess, ttLOrEq, ttGreater, ttGrOrEq,
						   ttEqual, ttNotEqual,	ttBitAnd, ttBitXor, ttBitOr, ttLogicAnd, ttLogicOr,
						   ttOpenSquareBracket, ttClosingSquareBracket, ttDot, ttComma, ttAssign, ttType,
						   ttSemicolon, ttConst, ttOpenFigureBracket, ttClosingFigureBracket};

const string TokenTypeString[] = {"CharConst", "Delimiter", "EOF", "Identifier",
								  "IntegerConst", "Operation", "RealConst", "Reserved", "StringConst",
								  "Plus", "Minus", "Mul", "Div", "Opening bracket", "Closing bracket",
								  "Left shift", "Right Shift", "Less", "Less Or Equal", "Greater",
								  "Greater Or Equal", "Equal", "Not equal", "BitAND", "BitXOR",
								  "BitOR", "Logical AND", "Logical OR", "Opening square bracket",
								  "Closing square bracket", "Dot", "Comma", "Assignment", "Type", "Semicolon", "Const",
								  "Opening figure bracket", "Closing figure bracket"};

typedef enum					 {START, DIGIT, LETTER, OPERATION, MAYBE_EQUAL,
								  STRING, WAIT_STRING, CHAR, SYM_DIV, DELIM, HEX_DIGIT, REAL_DIGIT, EXPONENT,
								  SYM_DOT, EXPONENT_STEP_2, EXPONENT_STEP_3, COMMENT_LINE, COMMENT_BRACE,
								  OCTAL_DIGIT, MAYBE_dPLUS, MAYBE_dMINUS, MAYBE_dAMP, MAYBE_dVERT, MAYBE_SHL, MAYBE_SHR,
								  MAYBE_NOT_EQUAL, COMMA, MUL, CIRCUMFLEX, OPEN_BRACKET, CLOSING_BRACKET,
								  OPEN_SQUARE_BRACKET, CLOSING_SQUARE_BRACKET, SEMICOLON,
								  OPEN_FIGURE_BRACKET, CLOSING_FIGURE_BRACKET} SM_STATES;
//Classes:

class TokenBase
{
protected:
	friend class Scanner;
	TokenType type;
	string text;
	unsigned int line, pos;
public:
	TokenBase(TokenType t, const string& str, int l, int p) : type(t), text(str), line(l), pos(p) {}
	virtual ~TokenBase(){}

	virtual void PrintToken(ostream& os)
	{
		os << "[" << line << "," << pos << "] " << TokenTypeString[type] << " " << text << endl;
	}
};

template <typename TokenValue>
class Token : public TokenBase
{
	friend class Scanner;
	TokenValue value;
public:                                                                                                                                                                                                                                              
	Token(TokenType t, const string& str, const TokenValue& val, int l, int p):
		TokenBase(t, str, l, p), value(val) {}
	~Token(){}
	Token(const Token& tok) : TokenBase(tok), value(tok.value) {}

	TokenValue GetValue() {return value;}
	  
	virtual void PrintToken(ostream& os)
	{
		  os << "[" << line << "," << pos << "]" << " " << TokenTypeString[type] << " " << text << "  " << value << endl;		  
	}
};

class Scanner
{
	ifstream input;
	stack <char> charstack;	
public:
	void NextToken();

	Scanner(const char *str) : input(str), tok(0), line(1), pos(1), pline(1), ppos(1)
	{
		if (input.fail()) Error::GenSimpleError("Error while opening source file");
	}
	~Scanner()
	{
		input.close();
		if (tok) delete tok;
	}

	template <typename T> void MakeToken(TokenType t, const string& str, const T& val);
	template <typename T> T ReadString (const string& str);

	bool isEof () {return input.eof() && charstack.empty();}
	bool CheckToken (TokenType type) {return tok->type == type;}

	void MakeToken(TokenType t, const string& str);
	void PutChar (char ch);

	int GetNumber (const string& str, int param);

	char GetChar ();

	string GetText() {return tok->text;}
	string ReadString (size_t start, const string& str);

	TokenBase& GetToken() {return *tok;}
	TokenType GetTokenType () {return tok->type;}

	unsigned int line, pos;
	unsigned int pline, ppos;

	char ch;
	int counter;

	SM_STATES state, pstate;
	size_t start_for_string;
	TokenBase* tok;

	string tmp;
	string current_str;
};

	