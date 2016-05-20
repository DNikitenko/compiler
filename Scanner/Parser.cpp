#include "Parser.h"
/////////////////////////////////////////////////////////////////////////
/*
Priorities:
1. ! + - (unary)
2. * /
3. + -
4. << >>
5. < <= >= >
6. == !=
7. &
8. ^
9. |
10. &&
11. ||
12. =

*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
Expr * Parser::ParseFuncCall(FuncCall * fc)
{
	vector <Expr *> arg_list;

	scn->NextToken();
	if (scn->GetTokenType() == ttClosingBracket)
	{
		if (!fc->GetValue()->ReturnTable()->params.empty()) CallException(5);
		scn->NextToken();

		return fc;
	}

	for (size_t i = 0; i < fc->GetValue()->ReturnTable()->params.size(); i++)
	{
		Expr * ptr = ParseExpr();

		SymbolType * t_par = fc->GetValue()->ReturnTable()->params[i]->ReturnVarType()->ReturnType();
		SymbolType * t_expr = ptr->ReturnSymbolType()->ReturnType();

		if (fc->ReturnName() != "printf")
		{
			if (t_par->ValueTypeFloat() && t_expr->ValueTypeInt())
				ptr = new UnOp ("i2f", ptr);
			else if (t_par->ValueTypeInt() && t_expr->ValueTypeFloat())
				ptr = new UnOp ("f2i", ptr);
			else if (!(t_par->ValueTypeFloat() && t_expr->ValueTypeFloat() || t_par->ValueTypeInt() && t_expr->ValueTypeInt()
				||t_par->StringTest() && t_expr->StringTest() || t_par->StructTest() && t_expr->StructTest())) CallException(6);
		}
		arg_list.push_back(ptr);

		if (i != fc->GetValue()->ReturnTable()->params.size()-1)
		{
			if (scn->GetTokenType() != ttComma) CallException(7);
			scn->NextToken();
		}
	}

	if (scn->GetTokenType () != ttClosingBracket) CallException(8);

	scn->NextToken();
	fc->SetArgList(arg_list);

	return fc;
}

Expr * Parser::ParseFactor()
{
	Expr *ptr;
	if (scn->GetTokenType () == ttOpenBracket)
	{
		scn->NextToken();
		ptr = ParseAssignment();

		if (scn->GetTokenType () != ttClosingBracket) CallException(8);

		scn->NextToken();

		return ptr;
	}

	if (scn->GetTokenType () == ttPlus || scn->GetTokenType () == ttMinus || scn->GetText() == "!" || scn->GetTokenType () == ttBitAnd)
	{
		string t = scn->GetText();

		scn->NextToken();

		return new UnOp (t, ParseFactor());
	}

	if (scn->GetTokenType () == ttInt)
	{
		if (scn->state == HEX_DIGIT)
			ptr = new IntegerConstant(scn->GetNumber (scn->GetText(), 16),
				static_cast <SymbolTypeInteger *> (cur_tbl->GetSymbol("int", scn)));
		else if (scn->state == OCTAL_DIGIT)
			ptr = new IntegerConstant(scn->GetNumber (scn->GetText(), 8),
				static_cast <SymbolTypeInteger *> (cur_tbl->GetSymbol("int", scn)));
		else ptr = new IntegerConstant(scn->ReadString <int> (scn->GetText()),
				static_cast <SymbolTypeInteger *> (cur_tbl->GetSymbol("int", scn)));

		scn->NextToken();

		return ptr;
	}

	if (scn->GetTokenType () == ttReal)
	{
		ptr = new FloatConstant(scn->ReadString <float> (scn->GetText()), 
				static_cast <SymbolTypeFloat *> (cur_tbl->GetSymbol("float", scn)));

		scn->NextToken();

		return ptr;
	}

	if (scn->GetTokenType () == ttString)
	{
		ptr = new StringConstant(scn->GetText(), new SymbolTypeString ());
		scn->NextToken();
		return ptr;
	}

	if (scn->GetTokenType () == ttIdentifier)
	{
		string t = scn->GetText();
		Symbol * sym = cur_tbl->GetSymbol(t, scn);
		ptr = new Variable (t, static_cast <SymbolVar *> (sym));

		scn->NextToken();

		if (scn->GetTokenType () == ttOpenBracket)
		{
			if (t == "main") CallException(9);
			FuncCall * fc = new FuncCall (t, static_cast <SymbolFunc *> (sym));

			if (!fc->GetValue()->FuncTest()) CallException(10);

			ptr = ParseFuncCall(fc);
		}

		return ptr;
	}

	Error::GenParseError("Unexpected token " + scn->GetText(), scn->line, scn->pos);
	return NULL;
}

Expr * Parser::ParseArrayAndStruct()
{
	SymbolType * type = NULL;
	Expr * ptr = ParseFactor();

	if (ptr->ReturnSymbolType()->VoidTest())
		type = static_cast <FuncCall *> (ptr)->ReturnSymbolType();

	else if (!ptr->ReturnSymbolType()->IntegerTest() && !ptr->ReturnSymbolType()->FloatTest())
		type = static_cast <Variable *> (ptr)->GetValue()->ReturnVarType();

	while (scn->GetTokenType () == ttOpenSquareBracket || scn->GetTokenType () == ttDot)
	{
		if (scn->GetTokenType () == ttOpenSquareBracket)
		{
			do
			{
				if (type == NULL) CallException(13);

				if (!type->ArrayTest()) CallException(13);

				scn->NextToken();
				ptr = new Array(ptr, ParseExpr(), cur_tbl, scn);
				type = static_cast <BinOp *> (ptr)->GetLeft()->ReturnSymbolType();
				ptr->SetType(type);

				if (scn->GetTokenType () != ttClosingSquareBracket) CallException(14);
				scn->NextToken();
			}
			while (scn->GetTokenType() == ttOpenSquareBracket);			
		}

		else // if "."

		{
			do
			{
				if (!type->StructTest() && !type->ArrayTest() &&
					static_cast <SymbolTypeArray *> (type)->ElemStructTest())
						CallException(16);
						
				SymbolTable * tmp = cur_tbl;
				CreateChild();			

				if (type->ArrayTest())
					cur_tbl = static_cast <SymbolTypeArray *> (type)->ReturnTable();
				else
					cur_tbl = static_cast <SymbolTypeStruct *> (type)->ReturnTable();

				cur_tbl->AddAncestor(tmp);

				scn->NextToken();
				ptr = new Struct(ptr, ParseArrayAndStruct(), cur_tbl, scn);	
				type = static_cast <BinOp *> (ptr)->GetLeft()->ReturnSymbolType();
				ptr->SetType(type);

				cur_tbl = tmp;
			}
			while (scn->GetTokenType() == ttDot);
		}
	}
	return ptr;
}

Expr * Parser::ParseTerm()
{
	Expr * ptr = ParseArrayAndStruct();
	while (scn->GetTokenType () == ttMul || scn->GetTokenType () == ttDiv)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp (t, ptr, ParseArrayAndStruct(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseShift()
{
	Expr *ptr = ParseTerm();
	while ((scn->GetTokenType ()) == ttPlus || scn->GetTokenType () == ttMinus)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseTerm(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseCompare()
{
	Expr *ptr = ParseShift();
	while ((scn->GetTokenType ()) == ttLShift || scn->GetTokenType () == ttRShift)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseShift(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseEq()
{
	Expr *ptr = ParseCompare();
	while (scn->GetTokenType () == ttLess     || scn->GetTokenType () == ttLOrEq ||
		   scn->GetTokenType () == ttGreater  || scn->GetTokenType () == ttGrOrEq)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseCompare(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseBitAND()
{
	Expr *ptr = ParseEq();
	while ((scn->GetTokenType ()) == ttEqual || scn->GetTokenType () == ttNotEqual)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseEq(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseBitXOR()
{
	Expr *ptr = ParseBitAND();
	while ((scn->GetTokenType ()) == ttBitAnd)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseBitAND(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseBitOR()
{
	Expr *ptr = ParseBitXOR();
	while ((scn->GetTokenType ()) == ttBitXor)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseBitXOR(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseLogicAND()
{
	Expr *ptr = ParseBitOR();
	while ((scn->GetTokenType ()) == ttBitOr)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseBitOR(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseLogicOR()
{
	Expr *ptr = ParseLogicAND();
	while ((scn->GetTokenType ()) == ttLogicAnd)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseLogicAND(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseAssignment()
{
	Expr *ptr = ParseLogicOR();
	while ((scn->GetTokenType ()) == ttLogicOr)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new BinOp(t, ptr, ParseLogicOR(), cur_tbl, scn);
	}
	return ptr;
}

Expr * Parser::ParseExpr()
{
	Expr *ptr =ParseAssignment();
	while ((scn->GetTokenType ()) == ttAssign)
	{
		string t = scn->GetText();
		scn->NextToken();
		ptr = new AssignOp(ptr, ParseExpr(), cur_tbl, scn);
	}
	return ptr;
}
//////////////////////////////////////////////////////////////////////////////
void Parser::CreateChild()
{//Here we create new sym. table and connect it with its predecessor
	SymbolTable *tt = new SymbolTable();
	tt->AddAncestor(cur_tbl);
	cur_tbl->AddTable(tt);
	cur_tbl = tt;
	cur_tbl->Add(new SymbolTypeString(), scn);
	cur_tbl->Add(new SymbolTypeInteger(), scn);
	cur_tbl->Add(new SymbolTypeFloat(), scn);
	cur_tbl->Add(new SymbolTypeVoid(), scn);
	return;
}

Statement * Parser::ParseStatement(int level)
{
	if (!level) CallException(44);
	if (scn->GetText() == "while") return ParseStatementWhile(level);
	if (scn->GetText() == "do") return ParseStatementDoWhile(level);
	if (scn->GetText() == "for") return ParseStatementFor(level);
	if (scn->GetText() == "continue") return ParseStatementContinue();
	if (scn->GetText() == "break") return ParseStatementBreak();
	if (scn->GetText() == "return") return ParseStatementReturn();
	if (scn->GetText() == "if") return ParseStatementIfElse(level);
	if (scn->GetTokenType() == ttSemicolon)
	{
		scn->NextToken();
		return new Empty_Operator();
	}
	if (scn->GetTokenType() == ttType || TypeExists(cur_tbl, scn->GetText()))
	{
		ParseTail(level, scn->GetText());

		return ParseStatement(level);
	}
	if (scn->GetTokenType() == ttOpenFigureBracket)
	{
		scn->NextToken();
		CreateChild();
		level++;
		Statement * block = ParseStatementBlock(level);
		level--;
		cur_tbl = cur_tbl->GetAncestor();

		return block;
	}
	if (scn->GetTokenType() == ttClosingFigureBracket) return new Statement_Expr(NULL);
	else 
	{
		Statement_Expr * x = new Statement_Expr (ParseExpr());

		if (scn->GetTokenType () != ttSemicolon) CallException(18);

		scn->NextToken();

		return x;
	}
}

Statement_Block * Parser::ParseStatementBlock(int level)
{
	Statement_Block * block = new Statement_Block(level);
	do
	{
		block->Add(ParseStatement(level)); //TODO: check whether wrong
	}
	while (scn->GetTokenType() != ttClosingFigureBracket && scn->GetTokenType() != ttClosingBracket);

	scn->NextToken();
	return block;
}

Statement_While * Parser::ParseStatementWhile(int level)
{
	InsideTheCycl++;
	scn->NextToken();

	if (scn->GetTokenType() != ttOpenBracket) CallException(41);
	scn->NextToken();

	Expr * condition = ParseExpr();
	if (scn->GetTokenType() != ttClosingBracket) CallException(8);

	scn->NextToken();
	if (scn->GetTokenType() == ttSemicolon) {scn->NextToken(); return new Statement_While(NULL, condition);}

	Statement * st = ParseStatement(level+1);
	if (InsideTheCycl) InsideTheCycl--;

	return new Statement_While(st, condition);
}

Statement_Do_While * Parser::ParseStatementDoWhile(int level)
{
	InsideTheCycl++;
	scn->NextToken();

	level++;
	Statement * body = ParseStatement(level);
	level--;

	if (InsideTheCycl) InsideTheCycl--;

	if (scn->GetText() != "while") CallException(43);

	scn->NextToken();
	if (scn->GetTokenType() != ttOpenBracket) CallException(41);

	scn->NextToken();
	Expr * condition = ParseExpr();
	if (scn->GetTokenType() != ttClosingBracket) CallException(8);

	scn->NextToken();
	if (scn->GetTokenType() != ttSemicolon) CallException(18);
	scn->NextToken();

	return new Statement_Do_While(body, condition);
}

Statement_For * Parser::ParseStatementFor(int level)
{
	InsideTheCycl++;

	Expr * init;
	Expr * condition;
	Expr * expr_list;

	scn->NextToken();
	if (scn->GetTokenType() != ttOpenBracket) CallException(41);
	scn->NextToken();

	if (scn->GetTokenType() == ttSemicolon) {init = NULL; scn->NextToken();}
	else 
	{
		init = ParseExpr();
		if (scn->GetTokenType () != ttSemicolon) CallException(18);
		scn->NextToken();
	}

	if (scn->GetTokenType() == ttSemicolon) {condition = NULL; scn->NextToken();}
	else 
	{
		condition = ParseExpr();
		if (scn->GetTokenType () != ttSemicolon) CallException(18);
		scn->NextToken();
	}

	if (scn->GetTokenType() == ttClosingBracket) expr_list = NULL;
	else 
	{
		expr_list = ParseExpr();

		if (scn->GetTokenType() != ttClosingBracket) CallException(8);
	}

	scn->NextToken();

	level++;
	Statement * block = ParseStatement(level);
	level--;

	if (InsideTheCycl) InsideTheCycl--;

	return new Statement_For (init, condition, expr_list, block);
}

Statement_Continue * Parser::ParseStatementContinue()
{
	if (!InsideTheCycl) CallException(45);
	scn->NextToken();

	if (scn->GetTokenType () != ttSemicolon) CallException(18);

	scn->NextToken();

	return new Statement_Continue();
}

Statement_Break * Parser::ParseStatementBreak()
{
	if (!InsideTheCycl) CallException(46);
	scn->NextToken();

	if (scn->GetTokenType () != ttSemicolon) CallException(18);

	scn->NextToken();

	return new Statement_Break();
}

Statement_Return * Parser::ParseStatementReturn()
{
	if (VoidFuncExists(CurFuncName))
	{
		scn->NextToken();
		if (scn->GetTokenType () != ttSemicolon) CallException(18);

		scn->NextToken();
		ReturnCounter++;

		return new Statement_Return();
	}
	scn->NextToken();
	Expr * temp = ParseExpr();
	if (scn->GetTokenType () != ttSemicolon) CallException(18);

	scn->NextToken();
	ReturnCounter++;

	return new Statement_Return(temp);
}

Statement_If * Parser::ParseStatementIfElse(int level)
{
	InsideTheCycl++;
	Statement * st1 = NULL;
	Statement * st2 = NULL;
	scn->NextToken();

	if (scn->GetTokenType() != ttOpenBracket) CallException(41);

	scn->NextToken();
 	Expr * condition = ParseExpr();

	if (scn->GetTokenType() != ttClosingBracket) CallException(8);

	scn->NextToken();

	level++;
	st1 = ParseStatement(level);
	level--;

	if (scn->GetText() != "else")
		return new Statement_If(condition, st1);

	scn->NextToken();

	level++;
	st2 = ParseStatement(level);
	level--;

	return new Statement_If_Else(condition, st1, st2);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Parser::TypeExists(SymbolTable * table, string s)
{ //Here we check, is there such type
		if (table->GetTable().count(s))
		{
			if (table->GetTable()[s]->AliasTest() || table->GetTable()[s]->ArrayTest())
				return true;
			else if (table->GetTable()[s]->StructTest())
					return true;			
		}

		if (table->GetAncestor() == NULL) return false;

		do
		{
			table = table->GetAncestor();
			if (table->GetTable().count(s))
			{
				if (table->GetTable()[s]->AliasTest() || table->GetTable()[s]->ArrayTest())
					return true;
				else if (table->GetTable()[s]->StructTest())
						return true;			
			}
		}
		while (table->GetAncestor() != NULL);

		return false;
	}

bool Parser::PossibleStart()
{
	return (TypeExists(cur_tbl, scn->GetText()) || scn->GetTokenType() == ttConst || scn->GetText() == "typedef"
			|| scn->GetTokenType() == ttOpenFigureBracket || scn->GetTokenType() == ttClosingFigureBracket
			|| scn->GetTokenType() == ttType || scn->GetText() == "struct" || scn->GetTokenType() == ttIdentifier
			|| scn->GetText() == "while" || scn->GetText() == "do" || scn->GetText() == "for" || scn->GetText() == "if"
			|| scn->GetText() == "return" || scn->GetText() == "continue" || scn->GetText() == "break"
			|| scn->GetTokenType() == ttSemicolon || scn->GetTokenType() == ttInt || scn->GetTokenType() == ttString
			|| scn->GetTokenType() == ttReal || scn->GetTokenType() == ttOpenBracket
			|| scn->GetTokenType() == ttPlus || scn->GetTokenType() == ttMinus);
	// Possible start of statement or definition
}

void Parser::CheckRet(Statement * stmt)
{
	if (GetFirstTable()->GetTable().count(CurFuncName)													    &&
		(GetFirstTable()->GetTable()[CurFuncName]->FuncTest())											    &&
		(!(static_cast <SymbolFunc *> (GetFirstTable()->GetTable()[CurFuncName])->ReturnType()->VoidTest()))) 
		stmt->CheckReturnType(GetFirstTable(), CurFuncName, scn);
	return;
}

bool Parser::PossibleExprStart()
{
	return scn->GetTokenType() == ttIdentifier && !TypeExists(cur_tbl, scn->GetText()) || scn->GetText() == "while"
		|| scn->GetText() == "do" || scn->GetText() == "for" || scn->GetText() == "if"
		|| scn->GetText() == "return" || scn->GetText() == "continue" || scn->GetText() == "break"
		|| VoidFuncExists(scn->GetText()) || scn->GetTokenType() == ttSemicolon
		|| scn->GetTokenType() == ttInt || scn->GetTokenType() == ttReal || scn->GetTokenType() == ttString
		|| scn->GetTokenType() == ttOpenBracket || scn->GetTokenType() == ttPlus || scn->GetTokenType() == ttMinus;
}

bool Parser::MainFuncExists()
{
	if (GetFirstTable()->GetTable().count("main"))
		if (GetFirstTable()->GetTable()["main"]->FuncTest()) 
			return true;

	return false; //'main' must be present always, how to live without it?! :)
}

bool Parser::VoidFuncExists(string s)
{
	if (GetFirstTable()->GetTable().count(s))
		if (GetFirstTable()->GetTable()[s]->FuncTest())
			if (static_cast <SymbolFunc *> (GetFirstTable()->GetTable()[s])->ReturnType()->VoidTest()) return true;

	return false;
}

void Parser::ParseConst()
{	// if we are here, so we'' try to parse constants such as: 'const int a = 0;'
	scn->NextToken();
	if (scn->GetTokenType() != ttType) CallException(19);

	string const_type = scn->GetText();
	scn->NextToken();

	if (scn->GetTokenType() != ttIdentifier) CallException(17);

	if (scn->GetText() == "main") CallException(22); //'main' is also reserved for you know what ))

	string const_name = scn->GetText();
	scn->NextToken();
	
	if (scn->GetTokenType() != ttAssign) CallException(22);

	scn->NextToken();
	if (scn->GetTokenType() != ttInt && scn->GetTokenType() != ttReal) CallException(22);

	Symbol * sym;
	if (scn->GetTokenType() == ttInt) //Here are some transformations between  'int' and 'float':
		sym = new SymbolVarConstInt(const_name, static_cast <SymbolTypeFloat *> (cur_tbl->GetSymbol("int", scn)),
			scn->ReadString <int> (scn->GetText())); //create integer variable

	else if (scn->GetTokenType() == ttReal)
	{
		sym = new SymbolVarConstFloat(const_name, static_cast <SymbolTypeFloat *> (cur_tbl->GetSymbol("float", scn)),
			scn->ReadString <float> (scn->GetText())); //create float variable
	}

	else CallException(23);

	cur_tbl->Add(sym, scn);
	scn->NextToken();

	if (scn->GetTokenType() != ttSemicolon) CallException(18);

	scn->NextToken();
	return;
}

void Parser::ParseTypedefOrVarArray(string const_type, string id, bool VarArray, string var_name)
{						//Here we parse arrays; it also can be an alias
	vector <int> len_list; //it's a vector of array' dimensions, i.e a[1][2]...
	do
	{
		if (scn->GetTokenType() != ttOpenSquareBracket) CallException(18);

		scn->NextToken();
		if (scn->GetTokenType() != ttInt || static_cast <Token <int> &> (scn->GetToken()).GetValue() <= 0) CallException(25);

		int value = static_cast <Token <int> &> (scn->GetToken()).GetValue(); //Too complicate; my scanner should be better 
		scn->NextToken();

		if (scn->GetTokenType() != ttClosingSquareBracket) CallException(14);

		len_list.push_back (value);
		scn->NextToken();
	}
	while(scn->GetTokenType() == ttOpenSquareBracket);

	if (scn->GetTokenType() != ttSemicolon && scn->GetTokenType() != ttComma) CallException(26);

	SymbolType *t;
	Symbol * sym = cur_tbl->GetSymbol(const_type, scn);

	if (!sym->TypeTest()) CallException(27);
	else t = static_cast <SymbolType *> (sym);

	if (!VarArray)
	{
		for (int i = len_list.size() - 1; i >= 0; i--)
			t = new SymbolTypeArray (id, t, len_list[i]);

		cur_tbl->Add(t, scn);
		scn->NextToken();
	}
	else
	{
		Symbol * sym;
		for (int i = len_list.size() - 1; i >= 0; i--)
			t = new SymbolTypeArray ("", t, len_list[i]);

		if (!BracketsCounter) sym = new SymbolVarGlobal(var_name, t);
		else if (CurStructName == "") sym = new SymbolVarLocal(var_name, t, GetVarOffset(CurFuncName, t->GetSize()));
		else sym = new SymbolVarLocal(var_name, t, GetStructVarOffset(CurStructName, t->GetSize()));

		cur_tbl->Add(sym, scn);
	}

	return;
}

void Parser::ParseStruct() //Here we try to parse structure
{
	scn->NextToken();
	if (scn->GetTokenType() != ttIdentifier) CallException(17);

	string name = scn->GetText();
	scn->NextToken();
	if (scn->GetTokenType() != ttOpenFigureBracket) CallException(29);
	scn->NextToken();

	CreateChild();

	SymbolTypeStruct * t = new SymbolTypeStruct(name, cur_tbl);

	CurStructName = name;
	StructOffsets[name] = 0;

	BracketsCounter++;
	do
	{
		SymbolTable * x = cur_tbl;
		//cur_tbl = t->ReturnTable();
		ParseTail (1, scn->GetText());
		cur_tbl = x;
	}
	while (scn->GetTokenType() == ttType && scn->GetText() != "void" || TypeExists(t->ReturnTable(), scn->GetText()));
	BracketsCounter--;
	CurStructName = "";

	if (scn->GetTokenType() != ttClosingFigureBracket) CallException(31);

	scn->NextToken();
	if (scn->GetTokenType() != ttSemicolon) CallException(18);

	scn->NextToken();
	cur_tbl = cur_tbl->GetAncestor();
	cur_tbl->Add(t, scn);	//Add parsed structure to the current symbol table

	return;
}
void Parser::ParseTypedef()
{
	scn->NextToken();
	if (scn->GetTokenType() != ttType) CallException(32);

	string name = scn->GetText();
	scn->NextToken();
	if (scn->GetTokenType() != ttIdentifier) CallException(17);

	string id = scn->GetText();

	scn->NextToken();
	if (scn->GetTokenType() == ttOpenSquareBracket)
	{
		ParseTypedefOrVarArray(name, id, false, ""); //here we wanna create array alias
		return;
	}

	Symbol * sym = cur_tbl->GetSymbol(name, scn);
	if (!sym->TypeTest()) CallException(33); //all these macro are to reduce the source code
	else
	{
		SymbolType * t = static_cast <SymbolType *> (sym);
		sym = new SymbolTypeAlias (id, t, t->GetSize());		// Here we create the alias of already existing type
	}

	cur_tbl->Add(sym, scn);

	if (scn->GetTokenType() != ttSemicolon) CallException(18);

	scn->NextToken();
	return;
}

SymbolFunc * Parser::ParseArguments(string name, string s, bool lval)
{
	SymbolFunc * sym_f = new SymbolFunc (s, lval);

	first_table->Add(sym_f, scn); // here we add the name of our function to the main sym. table
	scn->NextToken();

	if (scn->GetTokenType() == ttClosingBracket) // if function has not arguments
	{
		scn->NextToken();
		return sym_f;
	}

	if ((scn->GetTokenType() != ttType || scn->GetText() == "void") && !TypeExists(cur_tbl, scn->GetText())) CallException(34);

	string var_type = scn->GetText();

	while (scn->GetTokenType() == ttType && scn->GetText() != "void" || TypeExists(cur_tbl, scn->GetText()))
	{
		string var;
		bool isFormal = false;

		scn->NextToken();

		if (scn->GetTokenType() == ttBitAnd) // means that the parameter is formal
		{
			isFormal = true;
			scn->NextToken();
		}

		if (scn->GetTokenType() != ttIdentifier) CallException(17);
		var = scn->GetText();
		
		SymbolType * t;
		Symbol * sym = cur_tbl->GetSymbol(var_type, scn);

		if (!sym->TypeTest()) CallException(33);
		else t = static_cast <SymbolType *> (sym);

		SymbolVarParam * symVP = new SymbolVarParam (var, t, isFormal, GetVarOffset(CurFuncName, t->GetSize()));
		SymbolTable * table = sym_f->ReturnTable();						//Here we add all the params to the func's own table
		table->Add(symVP, scn);
		static_cast <SymbolTableFunc *> (table)->params.push_back(symVP);

		scn->NextToken();

		if (scn->GetTokenType() == ttClosingBracket) break;
		else if (scn->GetTokenType() == ttComma) scn->NextToken();
		else CallException(26);
	}
	scn->NextToken();

	return sym_f;
}

void Parser::ParseFuncDecl(string name, string s, bool FuncIsLValue)
{
	SymbolFunc * func;

	if (name == "void")
	{
		func = ParseArguments(name, s, FuncIsLValue);
		func->SetType(static_cast <SymbolTypeVoid *> (cur_tbl->GetSymbol("void", scn)));
	}

	else if (name == "int" || name == "float"  || name == "string" || TypeExists(cur_tbl, name))
	{
		SymbolType * t;
		func = ParseArguments(name, s, FuncIsLValue); // Parsing func's arguments
		Symbol * sym = cur_tbl->GetSymbol(name, scn);

		if (!sym->TypeTest()) CallException(35);
		else t = static_cast <SymbolType *> (sym);

		func->SetType(t);
	}

	else CallException(35);

	if (scn->GetTokenType() == ttOpenFigureBracket)
	{
		SymbolTable * t = func->ReturnEigenTable();
		t->AddAncestor(cur_tbl);

		cur_tbl = t;
		cur_tbl->Add(new SymbolTypeInteger(), scn);
		cur_tbl->Add(new SymbolTypeFloat(), scn);
		cur_tbl->Add(new SymbolTypeVoid(), scn);
		cur_tbl->Add(new SymbolTypeString(), scn);

		map <string, Symbol *>::iterator iter;
		map <string, Symbol *> tmp = func->ReturnTable()->GetTable();

		for (iter = tmp.begin(); iter != tmp.end(); iter++)
			cur_tbl->Add((*iter).second, scn); //Here we add local parameters to the function's symbol table

		scn->NextToken();

		int temp = BracketsCounter;
		vector <Statement *> vec;
		oper_list[CurFuncName] = vec;  //Here we add new function to the map containing its operators' list

		ParseFile(temp + 1);
		BracketsCounter = temp;

		return;
	}

	if (scn->GetTokenType() != ttSemicolon) CallException(18);
	scn->NextToken();

	return;
}

void Parser::ParseVar(string tt, vector <string> var_list, int level)
{
	if (tt == "void") CallException(39);

	Symbol * sym = cur_tbl->GetSymbol(tt, scn);
	SymbolType * t = static_cast <SymbolType *> (sym);

	vector <string> :: iterator cur;
	for(cur = var_list.begin(); cur < var_list.end(); cur++)
	{
		if (level) 
		{
			if (CurStructName == "")
				sym = new SymbolVarLocal(*cur, t, GetVarOffset(CurFuncName, t->GetSize()));
			else
				sym = new SymbolVarLocal(*cur, t, GetStructVarOffset(CurStructName, t->GetSize()));
		}
		else sym = new SymbolVarGlobal(*cur, t);			//Adding new vars to current sym. table

		cur_tbl->Add(sym, scn);
	}	
}

void Parser::ParseTail(int level, string tt)
{
	bool LValueFunc = false;
	if (scn->GetText() == "struct")
	{
		ParseStruct();
		return;
	}

	if (PossibleExprStart()) // Possible expression
	{
		Statement * temp = ParseStatement(level);
		if (InsideTheCycl) InsideTheCycl--;

		temp->CheckExprLValue(scn);
		CheckRet(temp);
		oper_list[CurFuncName].push_back(temp);//Parsed statement is being added to the list

		return;
	}

	string name = scn->GetText();
	scn->NextToken();

	if (scn->GetTokenType() != ttIdentifier && scn->GetTokenType() != ttBitAnd) CallException(17);
	if (scn->GetTokenType() == ttBitAnd)
	{
		LValueFunc = true;
		scn->NextToken();
	}

	vector <string> var_list;
	string s;

	while (scn->GetTokenType() == ttIdentifier)
	{
		s = scn->GetText();
		if (TypeExists(cur_tbl, s))
			Error::GenParseError("\"" + s + "\"" " cannot be variable's name", scn->line, scn->pos);

		var_list.push_back(s);
		scn->NextToken();

		if (scn->GetTokenType() == ttOpenSquareBracket)
		{
			if (name == "void") CallException(37);

			ParseTypedefOrVarArray(name, s, true, s);		//Parse array
			var_list.pop_back();

			if (scn->GetTokenType() == ttComma) scn->NextToken();
			continue;
		}

		if (scn->GetTokenType() == ttOpenBracket)
		{
			CurFuncName = s;
			ParseFuncDecl(name, s, LValueFunc);			
			//Parse function's declaration
			if (level) CallException(38);

			return;
		}

		if (scn->GetTokenType() == ttSemicolon) break;
		else if (scn->GetTokenType() == ttComma)
		{
			scn->NextToken();	//if we have a list of vars separated with comma
			continue;
		}
		else Error::GenParseError("Unexpected token " + scn->GetText(), scn->line, scn->pos);
	}

	if (scn->GetTokenType() != ttSemicolon) CallException(18);

	ParseVar(tt, var_list, level);							//Parse local or global variable
	scn->NextToken();

	return;
}

void Parser::ParseFile(int level)
{
	if (level != 0) BracketsCounter = level;
	while (PossibleStart() || VoidFuncExists(scn->GetText()))
	{
		if (scn->GetTokenType() == ttOpenFigureBracket)
		{
			BracketsCounter++;
			CreateChild();
			scn->NextToken();
			level++;
			continue;
		}

		else if (scn->GetTokenType() == ttClosingFigureBracket)
		{
			BracketsCounter--;
			cur_tbl = cur_tbl->GetAncestor();
			scn->NextToken();
			level--;
			if (level == 0)
			{
				if (ReturnCounter == 0) CallException(47); //Function returned nothing
				else ReturnCounter = 0;	
				CurFuncName = "";
			}
			continue;
		}

		else if (scn->GetText() == "const") ParseConst();
		else if (scn->GetText() == "typedef") ParseTypedef();
		else ParseTail(level, scn->GetText()); //All the previous cases failed, maybe var, func or smth. else
	}

	if (BracketsCounter) CallException(40); // Brackets mismatch

	return;
}

void Parser::PrintOperators() // Prints the function's body
{
	map <string, vector <Statement *>>::iterator it;

	for (it = oper_list.begin(); it != oper_list.end(); it++)
	{
		cout << (*it).first << ":" << endl;
		vector <Statement *>::iterator it1;

		for (it1 = (*it).second.begin(); it1 != (*it).second.end(); it1++)
			(*it1)->PrintStatement(1);
	}
}

void Parser::SetLocalVarsSizes()
{
	map <string, Symbol *> temp = first_table->GetTable();
	map <string, Symbol *>::iterator it;

	for (it = temp.begin(); it != temp.end(); it++)
	{
		if ((*it).second->FuncTest())
		{
			SymbolTableFunc * x = static_cast <SymbolFunc *> ((*it).second)->ReturnTable();
			vector <SymbolVarParam *> vec = x->params;
			for (size_t i = 0; i < vec.size(); i++)
			{
				vec[i]->ReturnVarType()->GetSize();
			}
		}
	}
}

Parser::Parser(Scanner * scn1)
{
	scn = scn1;

	cur_tbl = new SymbolTable();
	cur_tbl->Add(new SymbolTypeInteger(), scn1);
	cur_tbl->Add(new SymbolTypeFloat(), scn1);
	cur_tbl->Add(new SymbolTypeVoid(), scn1);
	cur_tbl->Add(new SymbolTypeString(), scn1);

	SymbolFunc * getch_func = new SymbolFunc("getch", false); //Embedded function getch()
	getch_func->SetType(static_cast <SymbolTypeVoid *> (cur_tbl->GetSymbol("void", scn)));
	cur_tbl->Add(getch_func, scn1);

	//Embedded function printf()
	SymbolFunc * printf_func = new SymbolFunc("printf", false);
	printf_func->SetType(static_cast <SymbolTypeVoid *> (cur_tbl->GetSymbol("void", scn)));
	
	SymbolType * t1 = static_cast <SymbolType *> (cur_tbl->GetSymbol("string", scn1));
	SymbolVarParam * symVP1 = new SymbolVarParam("format_type", t1, false, 0);
	printf_func->ReturnTable()->Add(symVP1, scn);
	static_cast <SymbolTableFunc *> (printf_func->ReturnTable())->params.push_back(symVP1);

	//The type of 2nd parameter is fictive, it can be any
	SymbolType * t2 = static_cast <SymbolType *> (cur_tbl->GetSymbol("string", scn1));
	SymbolVarParam * symVP2 = new SymbolVarParam("output", t2, false, 0);
	printf_func->ReturnTable()->Add(symVP2, scn);
	static_cast <SymbolTableFunc *> (printf_func->ReturnTable())->params.push_back(symVP2);

	cur_tbl->Add(printf_func, scn1);

	first_table = cur_tbl;
	InsideTheCycl = BracketsCounter = ReturnCounter = 0;

	/*operators["+"] = 0;
	operators["-"] = 1;
	operators["*"] = 2;
	operators["/"] = 3;
	operators["<<"] = 4;
	operators[">>"] = 5;
	operators["&"] = 6;
	operators["|"] = 7;
	operators["^"] = 8;
	operators["&&"] = 9;
	operators["||"] = 10;
	operators["=="] = 11;
	operators["!="] = 12;
	operators[">"] = 13;
	operators["<"] = 14;
	operators[">="] = 15;
	operators["<="] = 16;*/
}

size_t Parser::GetVarOffset (string FuncName, size_t size_of_var)
{
	return (static_cast <SymbolFunc *> (first_table->GetTable()[CurFuncName])->GetVarOffset(size_of_var));
}

size_t Parser::GetStructVarOffset(string StructName, size_t size_of_var)
{
	size_t prev = StructOffsets[StructName];
	StructOffsets[StructName] += size_of_var;

	return prev;
}