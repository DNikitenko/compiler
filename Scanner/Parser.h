#include "Statement.h"

class Parser
{
	SymbolTable * cur_tbl;

	Expr * ParseArrayAndStruct();
	Expr * ParseTerm();
	Expr * ParseFactor();
	Expr * ParseShift();
	Expr * ParseCompare();
	Expr * ParseEq();
	Expr * ParseBitAND();
	Expr * ParseBitXOR();
	Expr * ParseBitOR();
	Expr * ParseLogicAND();
	Expr * ParseLogicOR();
	Expr * ParseFuncCall(FuncCall * fc);
	Expr * ParseAssignment();	

	void ParseConst();
	void ParseTypedef();
	void ParseTail(int func_param, string tt); //It may be function, variable or array
	void ParseVar(string tt, vector <string> var_list, int level);
	void ParseTypedefOrVarArray(string const_type, string name, bool VarArray, string var_name);
	void ParseFuncDecl(string s1, string s2, bool FuncIsLValue);
	void ParseStruct();

	SymbolFunc * ParseArguments(string s1, string s2, bool lvalue);
	SymbolTable * first_table;

	int ReturnCounter;
	int BracketsCounter;

	int InsideTheCycl;

	string CurFuncName;
	string CurStructName;

	Scanner * scn;

	map <string, vector <Statement *>> oper_list;
	map <string, size_t> LocalVarsSize;
	map <string, size_t> StructOffsets;

public:
	Parser (Scanner * scn1);  

	void ParseFile (int level);
	void CallException(int message) {Error::GenParseError(code[message], scn->line, scn->pos);}
	void CreateChild();
	void CheckRet(Statement * stmt);
	void PrintOperators();
	void SetLocalVarsSizes();

	bool MainFuncExists();
	bool TypeExists(SymbolTable * table, string s);
	bool VoidFuncExists(string s);
	bool PossibleStart();
	bool PossibleExprStart();

	SymbolTable * GetTable() {return cur_tbl;}
	SymbolTable * GetFirstTable() {return first_table;}
	
	Statement * ParseStatement(int level);
	Statement_Block * ParseStatementBlock(int level);
	Statement_While * ParseStatementWhile(int level);
	Statement_Do_While * ParseStatementDoWhile(int level);
	Statement_For * ParseStatementFor(int level);
	Statement_Continue * ParseStatementContinue();
	Statement_Return * ParseStatementReturn();
	Statement_Break * ParseStatementBreak();
	Statement_If * ParseStatementIfElse(int level);

	Expr * ParseExpr();
	
	map <string, vector <Statement *>> GetOperList() {return oper_list;}
	map <string, vector <Statement *>> * GetLinkToOperList() {return &oper_list;}
	
	size_t GetVarOffset(string FuncName, size_t size_of_var);
	size_t GetStructVarOffset (string StructName, size_t size_of_var);
};