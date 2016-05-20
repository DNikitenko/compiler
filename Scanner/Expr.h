#include "AsmCommands.h"

class Expr
{
public:
	virtual void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts) {}
	virtual void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases) {}
	virtual void GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases) {}
	virtual void PrintTree (int depth) = 0;
	virtual void SetType (SymbolType * t) {}
	virtual bool BinOpTest() {return false;}
	virtual bool UnOpTest () {return false;}
	virtual bool StructTest () {return false;}
	virtual bool Assign () = 0;
	virtual bool lvalue () = 0;
	virtual bool ArrayTest() {return false;}
	virtual bool IntConstTest() {return false;}
	virtual bool FloatConstTest() {return false;}
	virtual int Var () = 0;
	virtual SymbolType * ReturnSymbolType() = 0;
	virtual string ReturnName() {return "";}

	virtual bool LocalVarTest()  {return false;}
	virtual bool GlobalVarTest() {return false;}
	virtual size_t ReturnBaseOffset(string var_type) {return 0;}
	virtual size_t GetSize() {return 0;}

	void PrintIndention (int depth);
	void CheckLvalue (Scanner * scn);
	void Add(vector <AsmCmd *> * cmds, AsmCmd * command) {cmds->push_back(command);}
	string GenerateLabel (vector <string> * labels);

	virtual bool ValueTypeInt()     {return false;}
	virtual bool ValueTypeFloat()   {return false;}
	virtual Expr * FoldConst() {return this;}
	virtual Expr * ChangeMulDivToShift() {return this;}
	virtual Expr * ChangeDivToMul() {return this;}
	virtual Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map) {return this;}
};

class FuncCall : public Expr
{
	string name;
	vector <Expr *> arg_list;
	SymbolFunc * value;
public:
	FuncCall (const string& token, SymbolFunc * f) : name(token), value(f) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts);
	void SetArgList (vector <Expr *> vec) {arg_list = vec;}
	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree (int depth);
	bool Assign() {return false;}
	bool lvalue () {return value->lvalue();}
	int Var () {return 0;}
	SymbolType * ReturnSymbolType() {return value->ReturnType();}
	SymbolFunc * GetValue () {return value;}
	string ReturnName() {return name;}
	size_t GetSize() {return value->ReturnType()->GetSize();}

	bool ValueTypeInt()     {return value->ReturnType()->ValueTypeInt();}
	bool ValueTypeFloat()   {return value->ReturnType()->ValueTypeFloat();}

	Expr * FoldConst();
	Expr * ChangeMulDivToShift();
	Expr * ChangeDivToMul();
	Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};

class BinOp : public Expr
{
protected:
	string value;
	Expr * left;
	Expr * right;
	SymbolType * ExprType;
public:
	BinOp (const string& token, Expr * ptr1, Expr * ptr2, SymbolTable *tbl, Scanner * scn);

	virtual bool Assign() {return false;}
	void GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts);
	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree (int depth);
	void SetType (SymbolType * t) {ExprType = t;}

	bool CheckArrayDimension(Expr * pointer);
	bool lvalue () {return false;}
	bool BinOpTest() {return true;}
	size_t GetSize() {return ExprType->GetSize();}
	
	int Var () {return 0;}

	SymbolType * ReturnSymbolType() {return ExprType;}
	string ReturnValue() {return value;}
	
	Expr * GetLeft () {return left;}
	Expr * GetRight () {return right;}

	bool ValueTypeInt()     {return ExprType->ValueTypeInt();}
	bool ValueTypeFloat()   {return ExprType->ValueTypeFloat();}

	Expr * FoldConst();
	Expr * ChangeMulDivToShift();
	Expr * ChangeDivToMul();
	virtual Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};

class AssignOp : public BinOp
{
public:
	AssignOp (Expr * ptr1, Expr * ptr2, SymbolTable * tbl, Scanner * scn) : BinOp ("=", ptr1, ptr2, tbl, scn) {}

	bool lvalue () {return false;}
	bool Assign() {return true;}

	Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};

class Variable : public Expr
{
	string name;
	SymbolVar * value;
public:
	Variable (string token, SymbolVar * sym) : value (sym), name(token) {}
	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree (int depth);
	bool lvalue () {return true;}
	bool Assign() {return false;}
	
	int Var () {return 1;}
	SymbolType * ReturnSymbolType() {return value->ReturnVarType();}
	SymbolVar * GetValue() {return value;}
	string ReturnName() {return name;}

	bool GlobalVarTest() {return value->GlobalVarTest();}
	bool LocalVarTest () {return value->LocalVarTest();}
	size_t GetSize() {return value->GetSize();}
	size_t GetArrayElemSize() {return value->GetSize();}
	void * ReturnVar() {return value;}

	bool ValueTypeInt()     {return value->ValueTypeInt();}
	bool ValueTypeFloat()   {return value->ValueTypeFloat();}

	Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};


class Array : public BinOp
{
public:
	Array (Expr * ptr1, Expr * ptr2, SymbolTable * tbl, Scanner * scn) : BinOp ("[]", ptr1, ptr2, tbl, scn) {}

	bool lvalue () {return true;}
	bool ArrayTest() {return true;}

	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);

	size_t ReturnBaseOffset(string var_type);
	string GetArrayName ();
	string ReturnName();
	
	Expr * GetLeft () {return left;}
	Expr * GetRight () {return right;}

	bool LocalVarTest();
	bool GlobalVarTest();
	size_t GetSize() {return ExprType->GetSize();}

	bool ValueTypeInt()     {return ExprType->ValueTypeInt();}
	bool ValueTypeFloat()   {return ExprType->ValueTypeFloat();}

	Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};

class UnOp : public Expr
{
	string value;
	Expr * op;
public:
	UnOp (const string& token, Expr * ptrl) : op(ptrl), value(token) {}

	void GenFloatAndStringAlias(vector <Symbol *> * vec,vector <AsmAllocDir *> * consts) 
		{op->GenFloatAndStringAlias(vec, consts);}
	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree (int depth);
	bool lvalue () /*{return op->lvalue();}*/ {return false;}
	bool UnOpTest() {return true;}
	bool BitAndTest() {return value == "&";}
	bool Assign() {return false;}
	int Var () {return 0;}
	SymbolType * ReturnSymbolType() {return op->ReturnSymbolType();}
	Expr * GetOp() {return op;}
	size_t GetSize() {return op->GetSize();}

	bool ValueTypeInt()     {return op->ValueTypeInt();}
	bool ValueTypeFloat()   {return op->ValueTypeFloat();}

	Expr * FoldConst();
	Expr * ChangeMulDivToShift();
	Expr * ChangeDivToMul();
	Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};

class IntegerConstant : public Expr
{
	int value;
	SymbolTypeInteger * ExprType;
public:
	IntegerConstant (int v, SymbolTypeInteger * t) : value (v), ExprType(t) {}

	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree(int depth);

	bool lvalue () {return false;}
	bool Assign() {return false;}

	int GetValue() {return value;}
	int Var () {return 0;}

	SymbolType * ReturnSymbolType() {return ExprType;}
	bool IntConstTest() {return true;}
	size_t GetSize() {return 4;}

	bool ValueTypeInt()     {return ExprType->ValueTypeInt();}
	bool ValueTypeFloat()   {return ExprType->ValueTypeFloat();}
};

class FloatConstant : public Expr
{
	float value;
	SymbolTypeFloat * ExprType;
	string ConstAlias;
public:
	FloatConstant (float v, SymbolTypeFloat * t) : value (v), ExprType (t) {ConstAlias = "";}

	void GenFloatAndStringAlias (vector <Symbol *> * vec, vector <AsmAllocDir *> * consts);
	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree (int depth);

	bool lvalue () {return true;}
	bool Assign() {return false;}
	
	int Var () {return 0;}
	SymbolType * ReturnSymbolType() {return ExprType;}
	size_t GetSize() {return 4;}	
	
	bool ValueTypeInt()     {return ExprType->ValueTypeInt();}
	bool ValueTypeFloat()   {return ExprType->ValueTypeFloat();}

	bool FloatConstTest() {return true;}
	float GetValue() {return value;}
};

class StringConstant : public Expr
{
	string value;
	SymbolTypeString * ExprType;
	string ConstAlias;
public:
	StringConstant (string v, SymbolTypeString * t) : value (v), ExprType (t) {value.erase(0, 1); value.erase(value.length()-1, 1);}
	void GenFloatAndStringAlias (vector <Symbol *> * vec, vector <AsmAllocDir *> * consts);
	void PrintTree (int depth);
	
	bool lvalue () {return true;}
	bool Assign() {return false;}
	
	int Var () {return 0;}
	SymbolType * ReturnSymbolType() {return ExprType;}
	string ReturnValue() {return value;}
	string GetConstAlias() {return ConstAlias;}
};

class Struct : public BinOp
{
public:
	Struct (Expr * ptr1, Expr * ptr2, SymbolTable * tbl, Scanner * scn) : BinOp(".", ptr1, ptr2, tbl, scn) {}

	bool StructTest() {return true;}
	void GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases);
	void PrintTree (int depth);
	bool lvalue () {return true;}
	bool Assign() {return false;}

	int Var () {return 0;}

	Expr * GetLeft () {return left;}
	Expr * GetRight () {return right;}

	string ReturnName() {return right->ReturnName();}
	string ReturnSourceName();
	SymbolType * ReturnSymbolType() {return ExprType;}
	
	bool LocalVarTest()  {return left->LocalVarTest();}
	bool GlobalVarTest() {return left->GlobalVarTest();}
	size_t GetSize() {return ExprType->GetSize();}

	bool ValueTypeInt()     {return right->ValueTypeInt();}
	bool ValueTypeFloat()   {return right->ValueTypeFloat();}

	Expr * DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map);
};