#include "LL_Optimizer.h"

class HL_Opt
{
	Parser * prs;
	Peephole * ll_opt;
	stack <map <string, VAR_TO_DUPLICATE>> const_map;
public:
	HL_Opt (Parser * prs_, Peephole * peep) : prs(prs_), ll_opt(peep) {}
	void Optimize();
};