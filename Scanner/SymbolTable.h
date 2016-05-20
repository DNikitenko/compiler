#include "Scanner.h"
 
class Symbol
{
public:
	Symbol (string t) : name(t){}

	string name;
	
	virtual ~Symbol(){}
	virtual bool Formal()        {return false;}
	virtual bool GlobalVarTest() {return false;}
	virtual bool LocalVarTest()  {return false;}
	virtual void PrintSymbol()   {}
	virtual void PushBackVarVector(vector <Symbol *> * consts) {}
	virtual void GenStructDef(vector <string> * var_vec) {}
	virtual int AliasTest()      {return 0;}
	virtual int ConstTest()      {return 0;}
	virtual int Var()            {return 0;}
	virtual int TypeTest()       {return 0;}
	virtual int FuncTest()	     {return 0;}
	virtual int IntegerTest()    {return 0;}
	virtual int FloatTest()      {return 0;}
	virtual int ArrayTest()      {return 0;}
	virtual int StructTest()     {return 0;}
	virtual int VoidTest()       {return 0;}
	virtual int StringTest()     {return 0;}
	virtual size_t GetStackOffset() {return 0;}
	virtual bool ValueTypeInt()     {return false;}
	virtual bool ValueTypeFloat()   {return false;}
	virtual bool ElemStructTest(){return false;}
	virtual size_t GetSize() {return 0;}
	virtual size_t GetArrayElemSize() {return 0;}
};

class SymbolType : public Symbol
{
protected:
	size_t bytes;
public:
	SymbolType (string name, size_t size) : Symbol(name), bytes(size) {}
	virtual ~SymbolType(){}

	virtual int AliasTest () {return 0;}
	virtual int IntegerTest () {return 0;}
	virtual int FloatTest () {return 0;}
	virtual string ReturnName() {return name;}
	virtual SymbolType * ReturnType () {return this;}

	void PushBackVarVector(vector <Symbol *> * consts) {}
	int TypeTest () {return 1;}
	size_t GetSize() {return bytes;}
};

class SymbolTypeFloat : public SymbolType
{
public:
	SymbolTypeFloat () : SymbolType("float", 4){}
	~SymbolTypeFloat(){}

	void PushBackVarVector(vector <Symbol *> * consts) {}
	void PrintSymbol () {cout << "float ";}
	int FloatTest () {return 1;}
	bool ValueTypeFloat() {return true;}
	size_t GetSize() {return bytes;}
};

class SymbolTypeInteger : public SymbolType
{
public:
	SymbolTypeInteger () : SymbolType ("int", 4) {}
	~SymbolTypeInteger(){}

	void PushBackVarVector(vector <Symbol *> * consts) {}
	void PrintSymbol () {cout << "int ";}
	int IntegerTest() {return 1;}
	bool ValueTypeInt() {return true;}
	size_t GetSize() {return bytes;}
};

class SymbolTypeString : public SymbolType
{
public:
	SymbolTypeString () : SymbolType("string", 0) {}
	~SymbolTypeString(){}

	void PushBackVarVector(vector <Symbol *> * consts) {}
	void PrintSymbol () {cout << "string ";}
	int StringTest() {return 1;}
};

class SymbolTypeVoid : public SymbolType
{
public:
	SymbolTypeVoid () : SymbolType("void", 0) {}
	~SymbolTypeVoid(){}

	void PushBackVarVector(vector <Symbol *> * consts) {}
	void PrintSymbol () {cout << "";}
	int VoidTest () {return 1;}
};

class SymbolTypeAlias : public SymbolType
{
	SymbolType *ref_type;
public:
	SymbolTypeAlias (string name, SymbolType *t, size_t s) : SymbolType(name, s), ref_type(t) {}
	~SymbolTypeAlias(){delete ref_type;}

	void PrintSymbol ();
	void PushBackVarVector(vector <Symbol *> * consts) {}

	int IntegerTest () {return ref_type->IntegerTest();}
	int FloatTest () {return ref_type->FloatTest();}
	int StructTest () {return ref_type->StructTest();}
	int AliasTest () {return 1;}

	bool ValueTypeInt() {return ref_type->ValueTypeInt();}
	bool ValueTypeFloat() {return ref_type->ValueTypeFloat();}

	SymbolType *GetRefType() {return ref_type;}
	SymbolType *ReturnType();

	size_t GetSize() {return ref_type->GetSize();}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SymbolVar : public Symbol
{
protected:
	SymbolType *type;
public:
	SymbolVar (string name, SymbolType * t) : Symbol(name), type(t) {}

	virtual ~SymbolVar() {delete type;}
	virtual bool isParam(){return false;}

	void PushBackVarVector(vector <Symbol *> * consts);
	int Var() {return 1;}
	SymbolType * ReturnVarType() {return type;}
	size_t GetSize() {return type->GetSize();}

	bool ValueTypeInt() {return type->ValueTypeInt();}
	bool ValueTypeFloat() {return type->ValueTypeFloat();}
};

class SymbolVarParam : public SymbolVar
{
	bool isFormal;
	size_t offset;
public:
	SymbolVarParam (string name, SymbolType * t, bool isFormal_, size_t off) : SymbolVar(name, t), isFormal(isFormal_), offset(off) {}
	~SymbolVarParam() {}

	bool Formal() {return isFormal;}
	bool isParam(){return true;}
	void PrintSymbol();
	size_t GetOffset() {return offset;}
	size_t GetSize() {return type->GetSize();}
};

class SymbolVarConstInt : public SymbolVar
{
	int value;
public:
	SymbolVarConstInt (string name, SymbolType *t, int val) : SymbolVar (name, t), value(val) {}
	~SymbolVarConstInt (){}

	void PrintSymbol ();
	int ConstTest() {return 1;}
};

class SymbolVarConstFloat : public SymbolVar
{
	double value;
public:
	SymbolVarConstFloat (string name, SymbolType *t, double val) : SymbolVar (name, t), value(val) {}
	~SymbolVarConstFloat (){}

	void PrintSymbol ();
	int ConstTest() {return 1;}
};

class SymbolVarLocal : public SymbolVar
{
	size_t offset;
public:
	SymbolVarLocal (string name, SymbolType * t, size_t off) : SymbolVar (name, t), offset(off) {}
	~SymbolVarLocal (){}

	size_t GetOffset() {return offset;}
	void PrintSymbol();
	bool LocalVarTest() {return true;}
	size_t GetSize() {return type->GetSize();}
};

class SymbolVarGlobal : public SymbolVar
{
public:
	SymbolVarGlobal (string name, SymbolType * t) : SymbolVar (name, t) {}
	~SymbolVarGlobal (){}

	bool GlobalVarTest() {return true;}
	void PrintSymbol();
	size_t GetSize() {return type->GetSize();}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SymbolTable
{
protected:
	map <string, Symbol *> table;
	vector <SymbolTable *> vec;
	SymbolTable * ancestor;
public:
	SymbolTable() {ancestor = NULL;}
	virtual ~SymbolTable() {}

	void Add (Symbol *sym, Scanner *scn);
	void PrintTable(int level);
	void AddTable(SymbolTable * t) {vec.push_back(t);}
	void AddAncestor (SymbolTable * t) {ancestor = t;}
	void SetTable (map <string, Symbol *> t) {table = t;}
	void ReturnVarsVector(vector <Symbol *> * vec);
	
	Symbol *GetSymbol (string name, Scanner *scn);
	map <string, Symbol *> GetTable () {return table;}
	SymbolTable * GetAncestor() {return ancestor;}
};

class SymbolTableFunc : public SymbolTable
{
public:
	SymbolTableFunc(){}
	~SymbolTableFunc(){}

	vector <SymbolVarParam *> params;
	void Addconsts (SymbolTable * st);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SymbolTypeStruct : public SymbolType
{
	SymbolTable * table;
public:
	SymbolTypeStruct (string name, SymbolTable * t) : SymbolType (name, 0), table (t) {}
	~SymbolTypeStruct() {delete table;}

	void PrintSymbol();
	void PrintSymbol(bool foo);
	void PrintStructName() {cout << name << " func ";}
	void GenStructDef(vector <string> * var_vec);
		
	int StructTest() {return 1;}

	SymbolTable * ReturnTable() {return table;}
	size_t GetSize();
};

class SymbolFunc : public Symbol
{
	SymbolTableFunc * table;
	SymbolTable * ftbl;
	SymbolType * type;
	Symbol * ReturnValue;
	size_t current_offset;
	string alias;
	bool lvalue_func;
public:
	SymbolFunc (string name, bool lval) : Symbol(name), table(new SymbolTableFunc()), ftbl(new SymbolTable), ReturnValue(NULL), lvalue_func(lval)
	{
		current_offset = 8; // Because when we call procedure, 8 bytes are busy
	}
	~SymbolFunc() {delete table; delete ftbl; delete type;}

	void PushBackVarVector(vector <Symbol *> * consts) {ftbl->ReturnVarsVector(consts);}
	void PrintSymbol();
	void SetType (SymbolType * t) {type = t;}

	int FuncTest() {return 1;}

	SymbolTableFunc * ReturnTable() {return table;}
	SymbolTable * ReturnEigenTable() {return ftbl;}
	SymbolType * ReturnType () {return type;}

	size_t GetVarOffset(size_t var_size);
	size_t GetStackOffset () {return current_offset;}
	void ChangeAlias (string als) {alias = als;}
	bool lvalue() {return lvalue_func;}

	int IntegerTest () {return lvalue_func ? ReturnValue->IntegerTest() : false;}
	int FloatTest () {return lvalue_func ? ReturnValue->FloatTest() : false;}
};


class SymbolTypeArray : public SymbolType
{
	SymbolType *elem_type;
	size_t length;
public:
	SymbolTypeArray (string name, SymbolType *elem_type_, int len) : elem_type(elem_type_),
		SymbolType(name, len*elem_type_->GetSize()), length(len) {}
	~SymbolTypeArray() {delete elem_type;}

	void PrintSymbol ();
	void PushBackVarVector(vector <Symbol *> * consts) {}
	int ArrayTest () {return 1;}
	bool ValueTypeInt() {return elem_type->ValueTypeInt();}
	bool ValueTypeFloat() {return elem_type->ValueTypeFloat();}

	SymbolType * ReturnElem_type () {return elem_type;}
	size_t ReturnLength () {return length;}
	size_t ReturnFullLength();
	SymbolType * GetVarType();
	size_t GetSize() { return length*elem_type->GetSize(); }
	size_t GetArrayElemSize()
	{
		if (elem_type->ArrayTest()) return static_cast <SymbolTypeArray *> (elem_type)->GetArrayElemSize();
		else return elem_type->GetSize();
	}
	SymbolTable * ReturnTable();
};
