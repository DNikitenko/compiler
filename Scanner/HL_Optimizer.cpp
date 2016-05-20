#include "HL_Optimizer.h"

#define MAX_HL_ITER 5

void HL_Opt::Optimize()
{
	map <string, vector <Statement *>> * opers;
	map <string, vector <Statement *>>::iterator it;

	map <string, VAR_TO_DUPLICATE> vrs;
	const_map.push(vrs);
	bool CodeChanged = false;

	opers = prs->GetLinkToOperList();

	for (int i = 0; i < MAX_HL_ITER; i++)
	for (it = (*opers).begin(); it != (*opers).end(); it++)
	{
		map <string, VAR_TO_DUPLICATE> vrs;
		const_map.push(vrs);

		for (size_t k = 0; k < (*it).second.size(); k++)
		{
			if ((*it).second[k]->StReturnTest() && (k+1) < (*it).second.size())
				(*it).second.erase((*it).second.begin()+k+1, (*it).second.end());

			if ((*it).second[k]->ZeroConditionTest())
				(*it).second.erase((*it).second.begin()+k);
		}

		for (size_t j = 0; j < (*it).second.size(); j++)
		{			
			(*it).second[j]->RemoveInaccessibleCode();
			(*it).second[j]->DuplicateConst(&const_map);
			(*it).second[j]->FoldConst();
			(*it).second[j]->ChangeMulDivToShift();
			(*it).second[j]->ChangeDivToMul();			
		}
	}

	ll_opt->GenOptimizedCode();
}