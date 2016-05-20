#include "Scanner.h"
#include "Cases.cpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char Scanner::GetChar ()
{
	char ch;
	if (charstack.empty())
	{
		input.read(&ch, 1);
		if (isEol(ch)) 
		{
			line++;
			pos = 1;
			return chNewLine;
		}
		else pos++;
	}
	else
	{
		ch = charstack.top();
		charstack.pop();
	}

	if (input.eof()) return 0;
	current_str += ch;
	return ch;
}

////////////////////////////////////////////////////////////////////////////////////////////

void Scanner::PutChar(char ch)
{
	current_str.erase(current_str.length() - 1);
	charstack.push(ch);
}

template <typename T>
T Scanner::ReadString(const string& str)
{
	T res;
	istringstream sstr(str);
	sstr >> res;

	return res;
}

int Scanner::GetNumber(const string& str, int param)
{
	int res;
	istringstream sstr(str);

	if (param == 16)
	{
		sstr >> hex >> res;
		return res;
	}

	if (param == 8)
	{
		sstr >> oct >> res;
		return res;
	}

	return NULL;
}
string Scanner::ReadString(size_t start, const string& str)
{
	string tmp = str.substr(start + 1, str.length() - start - 2);
	for (size_t b = tmp.find('\\'); b < tmp.length();)
	{
		char ch;
		switch (tmp[b+1])
		{
			case 'a' : ch = '\a'; break;
			case 'b' : ch = '\b'; break;
			case 'f' : ch = '\f'; break;
			case 'n' : ch = '\n'; break;
			case 'r' : ch = '\r'; break;
			case 't' : ch = '\t'; break;
			case 'v' : ch = '\v'; break;
			case '\\' : ch = '\\'; break;
			case '\'' : ch = '\''; break;
			case '\"' : ch = '\"'; break;
			case '\?' : ch = '\?'; break;
		}

		tmp.replace(b, b + 2, 1, ch);

		if (ch == '\\') b = tmp.find_first_not_of('\\');
		else b = tmp.find('\\');
	}

	return tmp;
}

template <typename T>
void Scanner::MakeToken(TokenType t, const string& str, const T& val)
{
	if (tok) delete tok;
	tok = new Token <T> (t, str, val, pline, ppos);
}

void Scanner::MakeToken(TokenType t, const string& str)
{
	if (tok) delete tok;
	tok = new TokenBase (t, str, pline, ppos);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void Scanner::NextToken()
{
	state = START, pstate = START;
	tmp = ""; 
	start_for_string = 0;
	counter = 0;

	while (1)
	{
		if (state == START)
		{
			current_str = "";
			ppos = pos;
			pline = line;
		}
		ch = GetChar();
		switch (state)
		{
		case START: 
			if (CASE_START_FUNC(this)) return;
			break;

		case DIGIT:
			if (CASE_DIGIT_FUNC(this)) return;
			break;

		case HEX_DIGIT:
			if (CASE_HEX_DIGIT_FUNC(this)) return;
			break;

		case OCTAL_DIGIT:
			if (CASE_OCTAL_DIGIT_FUNC(this)) return;
			break;

		case REAL_DIGIT:
			if (CASE_REAL_DIGIT_FUNC(this)) return;
			break;

		case EXPONENT:
			if (CASE_EXPONENT_FUNC(this)) return;
			break;

		case EXPONENT_STEP_2 :
			if (CASE_EXPONENT_STEP_2_FUNC(this)) return;	
			break;

		case EXPONENT_STEP_3 :
			if (CASE_EXPONENT_STEP_3_FUNC(this)) return;		
			break;

		case SYM_DOT :
			if (CASE_SYM_DOT_FUNC(this)) return;		
			break;		

		case LETTER:
			if (CASE_LETTER_FUNC(this)) return;		
			break;

		case SYM_DIV :
			if(CASE_SYM_DIV_FUNC(this)) return;		
			break;

		case DELIM :
			if(CASE_DELIM_FUNC(this)) return;		
			break;

		case COMMA:
			if(CASE_COMMA_FUNC(this)) return;		
			break;

		case SEMICOLON:
			if(CASE_SEMICOLON_FUNC(this)) return;
			break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////for 2 types of comments: // and /* */
		case COMMENT_LINE :
			if(CASE_COMMENT_LINE_FUNC(this)) return;		
			break;

		case COMMENT_BRACE :
			if(CASE_COMMENT_BRACE_FUNC(this)) return;		
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case STRING :                                                                                                                                                                                                                                                                                                                                                   
			if(CASE_STRING_FUNC(this)) return;	
			break;

		case WAIT_STRING :
			if(CASE_WAIT_STRING_FUNC(this)) return;		
			break; 

		case CHAR :
			if(CASE_CHAR_FUNC(this)) return;	
			break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		case MAYBE_EQUAL :
			if(CASE_MAYBE_EQUAL_FUNC(this)) return;		
			break;

		case MAYBE_dPLUS :
			if(CASE_MAYBE_dPLUS_FUNC(this)) return;	
			break;

		case MAYBE_dMINUS : 
			if(CASE_MAYBE_dMINUS_FUNC(this)) return;	
			break;

		case MAYBE_dAMP :
			if(CASE_MAYBE_dAMP_FUNC(this)) return;	    
			break;

		case MAYBE_dVERT :
			if(CASE_MAYBE_dVERT_FUNC(this)) return;	    	
			break;

		case MAYBE_SHL :
			if(CASE_MAYBE_SHL_FUNC(this)) return;	    	
			break;

		case MAYBE_SHR :
			if(CASE_MAYBE_SHR_FUNC(this)) return;	    	
			break;	
				
		case MAYBE_NOT_EQUAL :
			if(CASE_MAYBE_NOT_EQUAL_FUNC(this)) return;	    		
			break;

		case MUL:
			if(CASE_MUL_FUNC(this)) return;
			break;

		case CIRCUMFLEX:
			if(CASE_CIRCUMFLEX_FUNC(this)) return;
			break;

		case OPEN_BRACKET:
			if(CASE_OPEN_BRACKET_FUNC(this)) return;
			break;

		case CLOSING_BRACKET:
			if(CASE_CLOSING_BRACKET_FUNC(this)) return;
			break;

		case OPEN_SQUARE_BRACKET:
			if(CASE_OPEN_SQUARE_BRACKET_FUNC(this)) return;
			break;

		case CLOSING_SQUARE_BRACKET:
			if(CASE_CLOSING_SQUARE_BRACKET_FUNC(this)) return;
			break;

		case OPEN_FIGURE_BRACKET:
			if(CASE_OPEN_FIGURE_BRACKET_FUNC(this)) return;
			break;

		case CLOSING_FIGURE_BRACKET:
			if(CASE_CLOSING_FIGURE_BRACKET_FUNC(this)) return;
			break;
		}
	}
}