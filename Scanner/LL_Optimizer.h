#include "generator.h"

class Optimization
{
public:
	virtual void MakeOptimization (vector <AsmCmd *> * commands, size_t& i) {}
	virtual bool CanBeApplied (AsmCmd * line) {return false;}
	virtual bool CanBeApplied (AsmCmd * lines[2]) {return false;}
	virtual bool OneLine  () {return false;}
	virtual bool TwoLines () {return false;}
};

class OneLineOpt : public Optimization
{
public:
	virtual bool CanBeApplied (AsmCmd * line) {return false;}
	bool OneLine  () {return true;}
};

class TwoLinesOpt : public Optimization
{
public:
	virtual bool CanBeApplied (AsmCmd * lines[2]) {return false;}
	bool TwoLines  () {return true;}
};

class DeleteAddWithZero : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class DeleteSubWithZero : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class DeleteMultWithOne : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceCDQ : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceNEG : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceAddWithOne : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceSubWithOne : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceMovConst : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceCmpWithZero : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceMulToZero : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceIMUL : public OneLineOpt
{
public:
	bool CanBeApplied (AsmCmd * line);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class DeletePushPop : public TwoLinesOpt
{
public:
	bool CanBeApplied (AsmCmd * lines[2]);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplacePushPopWithMov : public TwoLinesOpt
{
public:
	bool CanBeApplied (AsmCmd * lines[2]);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class ReplaceMovPushWithPush : public TwoLinesOpt
{
public:
	bool CanBeApplied (AsmCmd * lines[2]);
	void MakeOptimization (vector <AsmCmd *> * commands, size_t& i);
};

class Peephole
{
	vector <AsmCmd *> commands;
	AsmCode * gen;
	AsmCmd * lines[2];
	vector <Optimization *> opts;

	bool code_changed;
public:
	Peephole (AsmCode * gen_);
	void OneLineOptimization();
	void TwoLinesOptimization();
	void GenOptimizedCode();
};