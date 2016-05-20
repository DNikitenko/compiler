#include "Parser.h"

class AsmCode
{
	bool ll_optimization;

	Parser * pars;
	ofstream output;

	vector <Symbol *> vec;     //Holds variables for GenerateDataSeg
	vector <Symbol *> vec_cpy; //"vec" copy for GenerateConstSeg

	map <string, vector <Statement *>> opers;
	map <string, string> func_aliases;

	vector <AsmCmd *> commands;
	vector <AsmAllocDir *> consts;
	vector <AsmAllocDir *> vars;
	vector <string> labels;
	stack <string> st;

	size_t init_index;

public:
	AsmCode (Parser * prs, const char * os, bool optimize) : pars(prs), output(os), ll_optimization(optimize), init_index(0) {}
	~AsmCode() {output.close();}
	void Generate ();
	void GenerateConstSeg(); //Creating float constants 
	void GenerateDataSeg ();
	void GenerateCodeSeg (bool to_file);
	void GenerateFuncAlias(string name);
	ofstream& GetOutput() {return output;}
	vector <AsmCmd *> GetCommands() {return commands;}
};