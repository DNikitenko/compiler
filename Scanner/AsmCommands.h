#include "SymbolTable.h"

class AsmCmd
{
protected:
	OPCODE opcode;
public:
	AsmCmd (OPCODE cmd) : opcode (cmd) {}
	virtual void GenerateToFile (ofstream& os);
	virtual bool AsmCmd1Test() {return false;}
	virtual bool AsmCmd2Test() {return false;}
	virtual bool OP_ADD_Test() {return opcode == OP_ADD ? true : false;}
	virtual bool OP_SUB_Test() {return opcode == OP_SUB ? true : false;}
	virtual bool OP_IMUL_Test() {return opcode == OP_IMUL ? true : false;}
	virtual bool OP_CDQ_Test() {return opcode == OP_CDQ ? true : false;}
	virtual bool OP_NEG_Test() {return opcode == OP_NEG ? true : false;}
	virtual bool OP_MOV_Test() {return opcode == OP_MOV ? true : false;}
	virtual bool OP_CMP_Test() {return opcode == OP_CMP ? true : false;}
	virtual bool OP_PUSH_Test() {return opcode == OP_PUSH ? true : false;}
	virtual bool OP_POP_Test() {return opcode == OP_POP ? true : false;}
	virtual bool TestRightOp(string pattern) {return false;}
	virtual string GetRightStringOp() {return "";}
	virtual REG  GetLeftReg () {return NOREG;}
	virtual REG  GetRightReg () {return NOREG;}
	virtual string GetLeftString() {return "";}
};

class AsmCmd1 : public AsmCmd
{
	string immed1;
protected:
	REG operand1;
public:
	AsmCmd1 (OPCODE op, REG oper1)   : AsmCmd(op), operand1(oper1), immed1("") {}
	AsmCmd1 (OPCODE op, string imm) : AsmCmd(op), operand1(NOREG), immed1(imm) {}
	AsmCmd1 (OPCODE op) : AsmCmd(op) {}
	virtual bool AsmCmd1Test() {return true;}
	REG  GetLeftReg () {return operand1;}
	string GetLeftString() {return immed1;}
	void GenerateToFile (ofstream& os);
};

class AsmCmd2 : public AsmCmd1
{
	string immed2;
	REG operand2;
	string immed3;
public:
	AsmCmd2 (OPCODE op, REG oper1, REG oper2) : AsmCmd1 (op, oper1), operand2(oper2), immed2("") {}
	AsmCmd2 (OPCODE op, REG oper1, string immed) : AsmCmd1(op, oper1), immed2(immed), operand2(NOREG) {}
	AsmCmd2 (OPCODE op, string immed, REG oper2) : AsmCmd1(op), immed2(immed), operand2(oper2) {operand1 = NOREG;}
	AsmCmd2 (OPCODE op, string imm1, string imm2) : AsmCmd1(op), immed2(imm1), immed3(imm2) {operand1 = operand2 = NOREG;}
	void GenerateToFile (ofstream& os);
	bool AsmCmd2Test() {return true;}
	bool TestRightOp(string pattern){return immed2 == pattern;}
	string GetRightStringOp() {return immed3 == "" ? immed2 : immed3;}
	string GetLeftString() {return immed3 == "" ? GetLeftString() : immed2;}
	REG  GetRightReg () {return operand2;}
};

class AsmAllocDir
{
protected:
	MEM_ALLOC dir;
	string name;
public:
	AsmAllocDir (MEM_ALLOC cmd, string var) : dir(cmd), name(var) {}
	virtual void GenerateToFile(ofstream& os) {}
};

class AsmStrucAlloc : public AsmAllocDir
{
	string type;
public:
	AsmStrucAlloc (MEM_ALLOC cmd, string name, string ty) : AsmAllocDir (cmd, name), type(ty) {}
	void GenerateToFile (ofstream& os);
};

class AsmUndefinedAlloc: public AsmAllocDir
{
public:
	AsmUndefinedAlloc(MEM_ALLOC cmd, string var) : AsmAllocDir(cmd, var) {}
	void GenerateToFile(ofstream& os);
};

class AsmResultPlace : public AsmAllocDir
{
public:
	AsmResultPlace(MEM_ALLOC cmd, string var) : AsmAllocDir(cmd, var) {}
	void GenerateToFile(ofstream& os);
};

class AsmIntAllocDir : public AsmAllocDir
{
	int value;
public:
	AsmIntAllocDir(MEM_ALLOC cmd, string var, int val) : AsmAllocDir(cmd, var),  value(val) {}
	void GenerateToFile(ofstream& os);
};

class AsmFloatAllocDir : public AsmAllocDir
{
	float value;
public:
	AsmFloatAllocDir(MEM_ALLOC cmd, string var, float val) : AsmAllocDir(cmd, var),  value(val) {}
	void GenerateToFile(ofstream& os);
};

class AsmStringAllocDir : public AsmAllocDir
{
	string value;
public:
	AsmStringAllocDir(MEM_ALLOC cmd, string var, string val) : AsmAllocDir(cmd, var),  value(val) {}
	void GenerateToFile(ofstream& os);
};

class AsmArrayAlloc : public AsmAllocDir
{
	size_t size;
public:
	AsmArrayAlloc(MEM_ALLOC cmd, string var, int size_) : 
	  AsmAllocDir (cmd, var), size(size_) {}
	void GenerateToFile (ofstream& os);
};

class AsmInvoke : public AsmCmd
{
	vector <string> arguments;
public:
	AsmInvoke(OPCODE op, vector <string> args) : AsmCmd(op), arguments(args) {}
	void GenerateToFile(ofstream& os);
};

class AsmLabel : public AsmCmd
{
	string label;
public:
	AsmLabel (string name) : AsmCmd(NOOPCODE), label(name) {}
	void GenerateToFile(ofstream& os);
};

class AsmMisc : public AsmCmd
{
	string text;
public:
	AsmMisc(string text_) : AsmCmd(NOOPCODE), text (text_) {}
	void GenerateToFile(ofstream& os);
};

class AsmProc : public AsmCmd //For MASM procedure directives
{
	string func_name;
public:
	AsmProc (OPCODE op, string f) : AsmCmd(op), func_name(f) {}
	void GenerateToFile(ofstream& os);
};