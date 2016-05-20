bool str_reserved (const string& str)
{
	for (int i = 0; i < (sizeof(Reserved) / sizeof(string) - 1); i++)
	{
		if (str.compare(Reserved[i]) == 0) return true;
	}
	return false;
}

bool str_TypeTest (const string& str)
{
	for (int i = 0; i < (sizeof(Reserved) / sizeof(string) - 1); i++)
	{
		if (str.compare(Type[i]) == 0) return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool isSpace (char ch)	   {return (ch >= 0) && (ch <= 32);}

bool isDigit (char ch)     {return (ch >= '0') && (ch <= '9');}

bool isLetter (char ch)    {return (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')));}

bool isHex (char ch)       {return (ch >= 'A') && (ch <= 'F') || (ch >= 'a') && (ch <= 'f');}

bool isOctal (char ch)     {return (ch >= '0') && (ch <= '7');}

bool isEol (char ch)       {return ch == chNewLine;}

bool isSymDot (char ch)	   {return ch == '.';}

bool isSymDiv (char ch)	   {return ch == '/' || ch == '*';}

bool isDelimiter (char ch) {return ch == ':' || ch == '?' || ch == '%' || ch == '~' || ch == '#';}

bool isComma (char ch) {return ch == ',';}
bool isSemicolon (char ch) {return ch == ';';}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CASE_START_FUNC (Scanner * const ptr)
{
	if (ptr->isEof())
			{			
				delete ptr->tok;
				ptr->tok = new TokenBase (ttEOF, "", ptr->line, ptr->pos);
				return 1;
			}
	if (isSpace(ptr->ch)) ptr->state = START;
			else if (ptr->ch == '\"') ptr->state = STRING;
			else if (ptr->ch == '\'') ptr->state = CHAR;
			else if (ptr->ch == '=') ptr->state = MAYBE_EQUAL;
			else if (ptr->ch == '+') ptr->state = MAYBE_dPLUS;
			else if (ptr->ch == '-') ptr->state = MAYBE_dMINUS;
			else if (ptr->ch == '&') ptr->state = MAYBE_dAMP;
			else if (ptr->ch == '|') ptr->state = MAYBE_dVERT;
			else if (ptr->ch == '<') ptr->state = MAYBE_SHL;
			else if (ptr->ch == '>') ptr->state = MAYBE_SHR;
			else if (ptr->ch == '!') ptr->state = MAYBE_NOT_EQUAL;
			else if (ptr->ch == '*') ptr->state = MUL;
			else if (ptr->ch == '^') ptr->state = CIRCUMFLEX;
			else if (ptr->ch == '(') ptr->state = OPEN_BRACKET;
			else if (ptr->ch == ')') ptr->state = CLOSING_BRACKET;
			else if (ptr->ch == '[') ptr->state = OPEN_SQUARE_BRACKET;
			else if (ptr->ch == ']') ptr->state = CLOSING_SQUARE_BRACKET;
			else if (ptr->ch == '{') ptr->state = OPEN_FIGURE_BRACKET;
			else if (ptr->ch == '}') ptr->state = CLOSING_FIGURE_BRACKET;
			else if (isDigit(ptr->ch)) ptr->state = DIGIT;
			else if (isLetter(ptr->ch) || ptr->ch == '_') ptr->state = LETTER;
			else if (isSymDot(ptr->ch)) ptr->state = SYM_DOT;
			else if (isSymDiv(ptr->ch)) ptr->state = SYM_DIV;
			else if (isDelimiter(ptr->ch)) ptr->state = DELIM;
			else if (isComma(ptr->ch)) ptr->state = COMMA;
			else if (isSemicolon(ptr->ch)) ptr->state = SEMICOLON;
			else
			{
				if (ptr->state == START) Error::GenScannError("Unknown lexeme", ptr->line, ptr->pos - 1);
				else if (ptr->state == DELIM) {ptr->MakeToken (ttDelimiter, ptr->current_str); return 1;}
			}
	return 0;
}

int CASE_DIGIT_FUNC (Scanner * const ptr)
{
			if (ptr->isEof() || !isDigit(ptr->ch) && (ptr->ch != 'x' && ptr->ch != 'X' || ptr->current_str[0] != '0')
												  &&  ptr->ch != '.' && ptr->ch != 'e' && ptr->ch != 'E')
			{
				if (isLetter(ptr->ch)) Error::GenScannError ("Wrong identifier", ptr->line, ptr->pos - 1);
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);							
				ptr->MakeToken <int> (ttInt, ptr->current_str, ptr->ReadString <int> (ptr->current_str));
				return 1;
			}
			else if (ptr->ch == '.') ptr->state = REAL_DIGIT;
			else if (ptr->ch == 'e' || ptr->ch == 'E') ptr->state = EXPONENT;
			else if (ptr->ch == 'x' || ptr->ch == 'X') ptr->state = HEX_DIGIT;
			else if (isOctal(ptr->ch) && ptr->current_str[0] == '0') ptr->state = OCTAL_DIGIT;
			else if (ptr->current_str[0] == '0') Error::GenScannError ("Wrong octal digit", ptr->line, ptr->pos - 1);
			return 0;
}

int CASE_HEX_DIGIT_FUNC (Scanner * const ptr)
{
			if (ptr->isEof() || !isDigit(ptr->ch) && !isHex(ptr->ch))
			{
				if (!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				size_t size = ptr->current_str.length() - 1;
				if (ptr->current_str[size] != 'x' && ptr->current_str[size] != 'X')
				{
					int x = 1, y = 0;
					for (size_t i = size; i > 1; i--)
					{
						y += (isDigit(ptr->current_str[i]) ? ptr->current_str[i] - '0' : ptr->current_str[i] - 'A' + 10 ) * x;
						x *= 16;
					}
					ptr->MakeToken <int> (ttInt, ptr->current_str, y);
					return 1;
				}
				else Error::GenScannError("Wrong hexadecimal digit", ptr->line, ptr->pos-1);
			}

			return 0;
}

int CASE_OCTAL_DIGIT_FUNC (Scanner * const ptr)
{
			if (ptr->isEof() || !isOctal(ptr->ch))
			{	
				if (!ptr->isEof() && !isDigit(ptr->ch) && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				if (isDigit(ptr->ch)) Error::GenScannError("Wrong octal digit", ptr->line, ptr->pos-1);

				int x = 1, y = 0;
				size_t size = ptr->current_str.length() - 1;
				for (size_t i = size; i >= 1; i--)
				{
					y += (ptr->current_str[i] - '0') * x;
					x *= 8;
				}
				ptr->MakeToken <int> (ttInt, ptr->current_str, y);
				return 1;
			}

			return 0;
}

int CASE_REAL_DIGIT_FUNC (Scanner * const ptr)
{
			if(ptr->isEof() || !isDigit(ptr->ch) && ptr->ch != 'e' && ptr->ch != 'E')
			{
				if (!isDigit(ptr->current_str[ptr->current_str.length() - 1]))
				{
					char temp = ptr->current_str[ptr->current_str.length() - 1];
					if (temp != ';' && temp != ',' && temp != ']' && temp != ')' && temp != '>' && temp != '<'
									&& temp != '&' && temp != '|' && temp != '*' && temp != '/' && temp != '+'
									&& temp != '-' && temp != 32  && temp != '&' && temp != '|' && temp != '^') 
						Error::GenScannError("Wrong real number", ptr->line, ptr->pos-1);
				}
				if (!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken <float> (ttReal, ptr->current_str, ptr->ReadString <float> (ptr->current_str));
				return 1;
			}
			else if (ptr->ch == 'e' || ptr->ch == 'E') ptr->state = EXPONENT;
			return 0;
}

int CASE_EXPONENT_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && (ptr->ch == '+' || ptr->ch == '-')) ptr->state = EXPONENT_STEP_2;
			else if(!ptr->isEof() && isDigit(ptr->ch)) ptr->state = EXPONENT_STEP_3;
			else Error::GenScannError ("Floating point token error", ptr->line, ptr->pos - 1);

			return 0;
}

int CASE_EXPONENT_STEP_2_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && isDigit(ptr->ch)) ptr->state = EXPONENT_STEP_3;
				else Error::GenScannError ("Floating point token error", ptr->line, ptr->pos - 1);

			return 0;
}

int CASE_EXPONENT_STEP_3_FUNC (Scanner * const ptr)
{
			if(ptr->isEof() || !isDigit(ptr->ch)) 
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken <float> (ttReal, ptr->current_str, ptr->ReadString <float> (ptr->current_str));
				return 1;
			}

			return 0;
}

int CASE_SYM_DOT_FUNC (Scanner * const ptr)
{
			if(isDigit(ptr->ch) && ptr->pstate != START) ptr->state = REAL_DIGIT;
			else 
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttDot, ptr->current_str);
                return 1;
			}		

			return 0;
}

int CASE_LETTER_FUNC (Scanner * const ptr)
{
			if ((ptr->isEof() || !isDigit(ptr->ch) && !isLetter(ptr->ch) && ptr->ch != '_'))
			{
					if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);

					if (ptr->current_str == "const")
						ptr->MakeToken (ttConst, ptr->current_str);
					else if (str_reserved(ptr->current_str))
						ptr->MakeToken (ttReserved, ptr->current_str);
					else if (str_TypeTest(ptr->current_str))
						ptr->MakeToken (ttType, ptr->current_str);
					else ptr->MakeToken(ttIdentifier, ptr->current_str);
					return 1;
			}

			return 0;
}

int CASE_SYM_DIV_FUNC (Scanner * const ptr)
{
			if (ptr->ch == '/') ptr->state = COMMENT_LINE;
			else if (ptr->ch == '*') ptr->state = COMMENT_BRACE;
			else 
			{
				if(ptr->isEof() || ptr->ch != '=')
				{
					if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
					ptr->MakeToken (ttDiv, ptr->current_str);
					return 1;
				}
				ptr->MakeToken (ttOperation, ptr->current_str);
				return 1;
			}

			return 0;
}

int CASE_DELIM_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttDelimiter, ptr->current_str);

	        return 1;
}

int CASE_COMMA_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttComma, ptr->current_str);

	        return 1;
}

int CASE_SEMICOLON_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttSemicolon, ptr->current_str);

	        return 1;
}

int CASE_COMMENT_LINE_FUNC (Scanner * const ptr)
{
			if(ptr->ch == chNewLine || ptr->isEof()) ptr->state = ptr->pstate;
			return 0;
}

int CASE_COMMENT_BRACE_FUNC (Scanner * const ptr)
{
			if(ptr->isEof()) Error::GenScannError("Comment must be finished", ptr->line, ptr->pos - 1);
			if(ptr->ch == '/' && ptr->current_str[ptr->current_str.length() - 2] == '*') ptr->state = ptr->pstate;

			return 0;
}

int CASE_STRING_FUNC (Scanner * const ptr)
{
			if(ptr->isEof()) Error::GenScannError("String constant must be finished", ptr->line, ptr->pos - 1);
			size_t size = ptr->current_str.length();

			if (ptr->ch == '"' && (ptr->current_str[size - 2] != '\\' || ptr->current_str[size - 3] == '\\') )
			{ 
					ptr->state = WAIT_STRING;
					ptr->tmp += ptr->ReadString (ptr->start_for_string, ptr->current_str);
			}

			return 0;
}

int CASE_WAIT_STRING_FUNC (Scanner * const ptr)
{
			if(ptr->isEof() || ptr->ch != '"' && !isSpace(ptr->ch) && ptr->ch != '/') 
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken <string> (ttString, ptr->current_str, ptr->tmp);
				return 1;
			} 
				
			else if (ptr->ch == '"')
			{
			      ptr->state = STRING;
			   	  ptr->start_for_string = ptr->current_str.length()-1;
				  ptr->PutChar(ptr->ch);
				  ptr->MakeToken <string> (ttString, ptr->current_str, ptr->tmp);
				  return 1;
			} 
				
			else if (ptr->ch == '/')
			{
				ptr->state = SYM_DIV;
				ptr->pstate = WAIT_STRING;
				ptr->PutChar(ptr->ch);
				ptr->MakeToken <string> (ttString, ptr->current_str, ptr->tmp);

				return 1;
			}

			return 0;
}

int CASE_CHAR_FUNC (Scanner * const ptr)
{
			if(ptr->isEof()) Error::GenScannError("Char constant must be finished", ptr->line, ptr->pos - 1);
			if(ptr->ch == '\'')
			{
				ptr->tmp = ptr->ReadString (0, ptr->current_str);
				if(ptr->tmp.length() == 1)
				{ 	
					ptr->MakeToken <char> (ttChar, ptr->current_str, ptr->tmp[0]);
					return 1;
				}
				else Error::GenScannError("Incorrect char value", ptr->line, ptr->pos);
			}

			return 0;
}

int CASE_MAYBE_EQUAL_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttEqual, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || ptr->ch != '=')
			{
				if (!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				if (ptr->current_str == "=") ptr->MakeToken (ttAssign, ptr->current_str);
				else ptr->MakeToken (ttOperation, ptr->current_str);
				return 1;
			}

			ptr->counter++;

			return 0;
}

int CASE_MAYBE_dPLUS_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttOperation, ptr->current_str);
				return 1;
			}

			if (ptr->isEof() || (ptr->ch != '+') && (ptr->ch != '='))
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttPlus, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MAYBE_dMINUS_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttOperation, ptr->current_str);
				return 1;
			}
		
			if(ptr->isEof() || (ptr->ch != '-') && (ptr->ch != '>') && (ptr->ch != '=') )
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttMinus, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MAYBE_dAMP_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttLogicAnd, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || (ptr->ch != '&') && (ptr->ch != '='))
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttBitAnd, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MAYBE_dVERT_FUNC (Scanner * const ptr)
{			
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttLogicOr, ptr->current_str);
				return 1;
			}
	
			if(ptr->isEof() || (ptr->ch != '|') && (ptr->ch != '='))
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttBitOr, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MAYBE_SHL_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttLShift, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || (ptr->ch != '<') && (ptr->ch != '='))
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);

				if (ptr->current_str == "<")
					ptr->MakeToken (ttLess, ptr->current_str);
				if (ptr->current_str == "<=")
					ptr->MakeToken (ttLOrEq, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MAYBE_SHR_FUNC (Scanner * const ptr)
{
			
			if (ptr->counter == 1)
			{
				if (!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);				
				ptr->MakeToken (ttRShift, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || (ptr->ch != '>') && (ptr->ch != '='))
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);

				if (ptr->current_str == ">")
					ptr->MakeToken (ttGreater, ptr->current_str);
				if (ptr->current_str == ">=")
					ptr->MakeToken (ttGrOrEq, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MAYBE_NOT_EQUAL_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttNotEqual, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || ptr->ch != '=')
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttOperation, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_MUL_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttOperation, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || ptr->ch != '=')
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttMul, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_CIRCUMFLEX_FUNC (Scanner * const ptr)
{
			if (ptr->counter == 1)
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttEqual, ptr->current_str);
				return 1;
			}

			if(ptr->isEof() || ptr->ch != '=')
			{
				if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
				ptr->MakeToken (ttBitXor, ptr->current_str);
				return 1;
			}
			ptr->counter++;

			return 0;
}

int CASE_OPEN_BRACKET_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttOpenBracket, ptr->current_str);

	        return 1;
}

int CASE_CLOSING_BRACKET_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttClosingBracket, ptr->current_str);

	        return 1;
}

int CASE_OPEN_SQUARE_BRACKET_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttOpenSquareBracket, ptr->current_str);

	        return 1;
}

int CASE_CLOSING_SQUARE_BRACKET_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttClosingSquareBracket, ptr->current_str);

	        return 1;
}

int CASE_OPEN_FIGURE_BRACKET_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttOpenFigureBracket, ptr->current_str);

	        return 1;
}

int CASE_CLOSING_FIGURE_BRACKET_FUNC (Scanner * const ptr)
{
			if(!ptr->isEof() && ptr->ch != chNewLine) ptr->PutChar(ptr->ch);
			ptr->MakeToken (ttClosingFigureBracket, ptr->current_str);

	        return 1;
}