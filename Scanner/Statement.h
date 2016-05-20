#include "Expr.h"
class Statement
{
protected:
	void PrintIndention (int depth);
public:
	virtual void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {}
	virtual void AddAsm(vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases) {}
	virtual void PrintStatement(int depth) {}
	virtual void CheckExprLValue(Scanner * scn) {}
	virtual void CheckReturnType(SymbolTable * t, string name, Scanner * scn) {}
	void Add(vector <AsmCmd *> * cmds, AsmCmd * command) {cmds->push_back(command);}
	virtual void FoldConst () {}
	virtual void ChangeMulDivToShift() {}
	virtual void ChangeDivToMul() {}
	virtual void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map) {}
	virtual void RemoveInaccessibleCode() {}
	virtual bool StReturnTest () {return false;}
	virtual bool StBreakTest () {return false;}
	virtual bool StContinueTest () {return false;}
	virtual bool StBlockTest () {return false;}
	virtual bool StExprTest()   {return false;}
	virtual bool ZeroConditionTest() {return false;}
};

class Empty_Operator : public Statement
{
public:
	void PrintStatement (int depth);
};

class Control_Operator : public Statement
{
public:
	virtual void PrintStatement (int depth) {}
};

class Statement_Expr : public Statement
{
	Expr * exp;
public:
	Statement_Expr(Expr * expP) : exp(expP) {}

	void PrintStatement (int depth) {if (exp) exp->PrintTree(depth);}
	void CheckExprLValue(Scanner * scn) {if (exp) exp->CheckLvalue(scn);}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
		{exp->GenFloatAndStringAlias(vec, consts);}

	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
		{exp->GenerateCode(commands, EAX, labels, st, func_aliases);}

	//Optimization:
	void FoldConst () {exp = exp->FoldConst();}
	void ChangeMulDivToShift() {exp = exp->ChangeMulDivToShift();}
	void ChangeDivToMul() {exp = exp->ChangeDivToMul();}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map){exp = exp->DuplicateConst(const_map);}
	bool StExprTest()   {return true;}
};

class Statement_Block : public Statement
{
	vector <Statement *> stmts;
	size_t level;
public:
	Statement_Block (size_t l) : level(l) {}

	bool StBlockTest () {return true;}
	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
	{
		vector <Statement *>::iterator it;
		for (it = stmts.begin(); it != stmts.end(); it++)
			(*it)->GenFloatAndStringAlias(vec, consts);
	}
	void PrintStatement(int depth);
	void CheckExprLValue (Scanner * scn);
	void Add (Statement *s) {stmts.push_back(s);}
	vector <Statement *> * GetStatements() {return &stmts;}
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void FoldConst()
	{
		vector <Statement *>::iterator it;
		for (it = stmts.begin(); it != stmts.end(); it++)
			(*it)->FoldConst();
	}
	void ChangeMulDivToShift()
	{
		vector <Statement *>::iterator it;
		for (it = stmts.begin(); it != stmts.end(); it++)
			(*it)->ChangeMulDivToShift();		
	}
	void ChangeDivToMul()
	{
		vector <Statement *>::iterator it;
		for (it = stmts.begin(); it != stmts.end(); it++)
			(*it)->ChangeDivToMul();	
	}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
	{
		map <string, VAR_TO_DUPLICATE> vrs;
		(*const_map).push(vrs);

		vector <Statement *>::iterator it;
		for (it = stmts.begin(); it != stmts.end(); it++)
			(*it)->DuplicateConst(const_map);
		(*const_map).pop();
	}
	void RemoveInaccessibleCode()
	{
		for (size_t k = 0; k < stmts.size(); k++)
		{
			if (stmts[k]->StReturnTest() && (k+1) < stmts.size())
				stmts.erase(stmts.begin()+k+1, stmts.end());
		}
	}
};

class Statement_While : public Statement
{
	Statement * stmt;
	Expr * condition;
public:
	Statement_While (Statement * stmt_, Expr * cnd) : stmt(stmt_), condition(cnd) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {if (stmt) stmt->GenFloatAndStringAlias(vec, consts); condition->GenFloatAndStringAlias(vec, consts);}
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void CheckExprLValue(Scanner * scn);
	void PrintStatement(int depth);
	void FoldConst () {condition = condition->FoldConst(); if (stmt) stmt->FoldConst();}
	void ChangeMulDivToShift() {condition->ChangeMulDivToShift(); if (stmt) stmt->ChangeMulDivToShift();}
	void ChangeDivToMul() {condition->ChangeDivToMul(); if (stmt) stmt->ChangeDivToMul();}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
	{
		//condition->DuplicateConst(const_map);

		map <string, VAR_TO_DUPLICATE> vrs;
		(*const_map).push(vrs);

		if (stmt) stmt->DuplicateConst(const_map);
		(*const_map).pop();
	}
	void RemoveInaccessibleCode()
	{
		if (stmt) stmt->RemoveInaccessibleCode();
		if (stmt && stmt->StBlockTest())
		{
			Statement_Block * block = static_cast <Statement_Block *> (stmt);
			vector <Statement *> * stmts = block->GetStatements();

			for (size_t k = 0; k < (*stmts).size(); k++)
			{
				if (((*stmts)[k]->StBreakTest() || (*stmts)[k]->StContinueTest()) && (k+1) < (*stmts).size())
					(*stmts).erase((*stmts).begin()+k+1, (*stmts).end());
			}
		}
	}
	bool ZeroConditionTest() {return (condition && condition->IntConstTest() && (!(static_cast <IntegerConstant *> (condition)->GetValue())));}
};

class Statement_If : public Statement
{
protected:
	Expr * condition;
	Statement * st1;
public:
	Statement_If (Expr * cnd, Statement * blck) : condition(cnd), st1(blck) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {condition->GenFloatAndStringAlias(vec, consts); if (st1) st1->GenFloatAndStringAlias(vec, consts);}
	virtual void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void CheckExprLValue(Scanner * scn);
	void PrintStatement(int depth);
	virtual void FoldConst () {condition = condition->FoldConst(); if (st1) st1->FoldConst();}
	virtual void ChangeMulDivToShift() {condition = condition->ChangeMulDivToShift(); if (st1) st1->ChangeMulDivToShift();}
	virtual void ChangeDivToMul() {condition = condition->ChangeDivToMul(); if (st1) st1->ChangeDivToMul();}
	virtual void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
	{
		//condition->DuplicateConst(const_map);

		map <string, VAR_TO_DUPLICATE> vrs;
		(*const_map).push(vrs);

		if (st1) st1->DuplicateConst(const_map);
		(*const_map).pop();
	}
	virtual void RemoveInaccessibleCode() {if (st1) st1->RemoveInaccessibleCode();}
	bool ZeroConditionTest() {return (condition && condition->IntConstTest() && (!(static_cast <IntegerConstant *> (condition)->GetValue())));}
};

class Statement_If_Else : public Statement_If
{
	Statement * st2;
public:
	Statement_If_Else(Expr * cnd, Statement * blck1, Statement * blck2) : 
	  Statement_If(cnd, blck1), st2(blck2) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {condition->GenFloatAndStringAlias(vec, consts); if (st1) st1->GenFloatAndStringAlias(vec, consts); if (st2) st2->GenFloatAndStringAlias(vec, consts);}
	void CheckExprLValue(Scanner * scn);
	void PrintStatement(int depth);
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void FoldConst () {condition = condition->FoldConst(); if (st1) st1->FoldConst(); if (st2) st2->FoldConst();}
	void ChangeMulDivToShift() {condition = condition->ChangeMulDivToShift(); if (st1) st1->ChangeMulDivToShift(); if (st2) st2->ChangeMulDivToShift();}
	void ChangeDivToMul() {condition = condition->ChangeDivToMul(); if (st1) st1->ChangeDivToMul(); if (st2) st2->ChangeDivToMul();}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
	{
		//condition->DuplicateConst(const_map);
		map <string, VAR_TO_DUPLICATE> vrs1, vrs2;

		(*const_map).push(vrs1);
		if (st1) st1->DuplicateConst(const_map);
		(*const_map).pop();

		(*const_map).push(vrs2);
		if (st2) st2->DuplicateConst(const_map);
		(*const_map).pop();
	}
};

class Statement_Do_While : public Statement
{
	Statement * stmt;
	Expr * condition;
public:
	Statement_Do_While(Statement * st, Expr * cnd) : stmt(st), condition(cnd) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {if (stmt) stmt->GenFloatAndStringAlias(vec, consts); condition->GenFloatAndStringAlias(vec, consts);}
	void CheckExprLValue(Scanner * scn);
	void PrintStatement(int depth);
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void FoldConst () {condition = condition->FoldConst(); if (stmt) stmt->FoldConst();}
	void ChangeMulDivToShift() {condition = condition->ChangeMulDivToShift(); if (stmt) stmt->ChangeMulDivToShift();}
	void ChangeDivToMul() {condition = condition->ChangeDivToMul(); if (stmt) stmt->ChangeDivToMul();}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
	{
		//condition->DuplicateConst(const_map); 
		if (stmt) stmt->DuplicateConst(const_map);
		
		map <string, VAR_TO_DUPLICATE> vrs;
		(*const_map).push(vrs);

		if (stmt) stmt->DuplicateConst(const_map);
		(*const_map).pop();
	}
	void RemoveInaccessibleCode()
	{
		if (stmt) stmt->RemoveInaccessibleCode();
		if (stmt && stmt->StBlockTest())
		{
			Statement_Block * block = static_cast <Statement_Block *> (stmt);
			vector <Statement *> * stmts = block->GetStatements();

			for (size_t k = 0; k < (*stmts).size(); k++)
			{
				if (((*stmts)[k]->StBreakTest() || (*stmts)[k]->StContinueTest()) && (k+1) < (*stmts).size())
					(*stmts).erase((*stmts).begin()+k+1, (*stmts).end());
			}
		}
	}
};

class Statement_For : public Statement
{
	Expr * init;
	Expr * condition;
	Expr * expr_list;
	Statement * body;
public:
	Statement_For (Expr * initial, Expr * cond, Expr * list, Statement * b) : 
	  init(initial), condition(cond), expr_list(list), body(b) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
	{
		if (init) init->GenFloatAndStringAlias(vec, consts);
		if (condition) condition->GenFloatAndStringAlias(vec, consts);
		if (expr_list) expr_list->GenFloatAndStringAlias(vec, consts);
		if (body) body->GenFloatAndStringAlias(vec, consts);
	}
	void CheckExprLValue(Scanner * scn);
	void PrintStatement(int depth);
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void FoldConst()
	{
		if (init) init = init->FoldConst();
		if (condition) condition = condition->FoldConst();
		if (expr_list) expr_list = expr_list->FoldConst();
		if (body) body->FoldConst();
	}
	void ChangeMulDivToShift()
	{
		if (init) init = init->ChangeMulDivToShift();
		if (condition) condition = condition->ChangeMulDivToShift();
		if (expr_list) expr_list = expr_list->ChangeMulDivToShift();
		if (body) body->ChangeMulDivToShift();	
	}
	void ChangeDivToMul()
	{
		if (init) init = init->ChangeDivToMul();
		if (condition) condition = condition->ChangeDivToMul();
		if (expr_list) condition = expr_list->ChangeDivToMul();
		if (body) body->ChangeDivToMul();	
	}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
	{
		//if (init) init->DuplicateConst(const_map);
		//if (condition) condition->DuplicateConst(const_map);
		//if (expr_list) expr_list->DuplicateConst(const_map);

		map <string, VAR_TO_DUPLICATE> vrs;
		(*const_map).push(vrs);

		if (body) body->DuplicateConst(const_map);

		(*const_map).pop();
	}
	void RemoveInaccessibleCode()
	{
		if (body) body->RemoveInaccessibleCode();
		if (body && body->StBlockTest())
		{
			Statement_Block * block = static_cast <Statement_Block *> (body);
			vector <Statement *> * stmts = block->GetStatements();

			for (size_t k = 0; k < (*stmts).size(); k++)//Removing code after "return"
			{
				if (((*stmts)[k]->StBreakTest() || (*stmts)[k]->StContinueTest()) && (k+1) < (*stmts).size())
					(*stmts).erase((*stmts).begin()+k+1, (*stmts).end());
			}
		}
	}
	bool ZeroConditionTest() {return (condition && condition->IntConstTest() && (!(static_cast <IntegerConstant *> (condition)->GetValue())));}
};

class Statement_Return : public Control_Operator
{
	Expr * ret;
	void PrintStatement(int depth);
public:
	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {if (ret) ret->GenFloatAndStringAlias(vec, consts);}
	void CheckReturnType(SymbolTable * t, string name, Scanner * scn);
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	void FoldConst() {if (ret) ret->FoldConst();}
	void ChangeMulDivToShift() {if (ret) ret->ChangeMulDivToShift();}
	void ChangeDivToMul() {if (ret) ret->ChangeDivToMul();}
	void DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map) {if (ret) ret->DuplicateConst(const_map);}
	bool StReturnTest () {return true;}

	Statement_Return() : ret(NULL) {}
	Statement_Return(Expr * r) : ret(r) {}
};

class Statement_Break : public Control_Operator
{
	void PrintStatement(int depth);
public:
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	bool StBreakTest () {return true;}
};

class Statement_Continue : public Control_Operator
{
	void PrintStatement(int depth);
public:
	void AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases);
	bool StContinueTest () {return true;}
};