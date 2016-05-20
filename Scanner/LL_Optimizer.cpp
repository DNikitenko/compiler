#include "LL_Optimizer.h"

Peephole::Peephole(AsmCode * gen_)
{
	gen = gen_;
	opts.push_back(new DeleteAddWithZero());
	opts.push_back(new DeleteSubWithZero());
	opts.push_back(new DeleteMultWithOne());
	opts.push_back(new ReplaceCDQ());
	opts.push_back(new ReplaceNEG());
	opts.push_back(new ReplaceAddWithOne());
	opts.push_back(new ReplaceSubWithOne());
	opts.push_back(new ReplaceMovConst());
	opts.push_back(new ReplaceCmpWithZero());
	opts.push_back(new ReplaceMulToZero());
	opts.push_back(new ReplaceIMUL());
	opts.push_back(new DeletePushPop());
	opts.push_back(new ReplacePushPopWithMov());
	opts.push_back(new ReplaceMovPushWithPush());
}

void Peephole::GenOptimizedCode()
{
	gen->GetOutput() << "include \\masm32\\include\\masm32rt.inc\n";
	gen->GetOutput() << ".686\n";

	gen->GenerateConstSeg();	  //Generates ".const" segment for float numbers and strings
	gen->GenerateDataSeg();
	gen->GenerateCodeSeg(false);

	OneLineOptimization();
	TwoLinesOptimization();

	for (size_t i = 0; i <= commands.size() - 1; i++)
		commands[i]->GenerateToFile(gen->GetOutput());
	gen->GetOutput() << "end start";
}

void Peephole::OneLineOptimization()
{
	commands = gen->GetCommands();
	for (size_t i = 0; i < commands.size(); i++)
	{
		for (size_t j = 0; j < opts.size(); j++)
		{
			if (opts[j]->OneLine() && opts[j]->CanBeApplied(commands[i]))
				opts[j]->MakeOptimization(&commands, i);			
		}
	}
}

void Peephole::TwoLinesOptimization()
{
	while (code_changed)
	{
		code_changed = false;
		for (int j = 0; j < 2; j++)
		{
			for (size_t i = j; (i+j) < commands.size()-1; i++)
			{
				lines[0] = commands[i];
				lines[1] = commands[i+1];

				for (size_t k = 0; k < opts.size(); k++)
				{
					if (opts[k]->TwoLines() && opts[k]->CanBeApplied(lines))
					{
						opts[k]->MakeOptimization(&commands, i);
						code_changed = true;
					}
				}				
			}
		}
	}
}

bool DeleteAddWithZero::CanBeApplied(AsmCmd * line)
{
	return line->OP_ADD_Test() && line->TestRightOp("0");
}

void DeleteAddWithZero::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->erase(commands->begin()+i--);
}

bool DeleteSubWithZero::CanBeApplied(AsmCmd * line)
{
	return line->OP_SUB_Test() && line->TestRightOp("0");
}

void DeleteSubWithZero::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->erase(commands->begin()+i--);
}

bool DeleteMultWithOne::CanBeApplied(AsmCmd * line)
{
	return line->OP_IMUL_Test() && line->TestRightOp("1");
}

void DeleteMultWithOne::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->erase(commands->begin()+i--);
}

bool ReplaceCDQ::CanBeApplied(AsmCmd * line)
{
	return line->OP_CDQ_Test();
}

void ReplaceCDQ::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->insert(commands->begin()+i++, new AsmCmd2(OP_MOV, EDX, EAX));
	commands->insert(commands->begin()+i++, new AsmCmd2(OP_SAR, EDX, "31"));
	commands->erase(commands->begin()+i--);
}

bool ReplaceNEG::CanBeApplied(AsmCmd * line)
{
	return line->OP_NEG_Test();
}

void ReplaceNEG::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->insert(commands->begin()+i++, new AsmCmd2(OP_XOR, (*commands)[i]->GetLeftReg(), "-1"));
	commands->insert(commands->begin()+i++, new AsmCmd1(OP_INC, (*commands)[i]->GetLeftReg()));
	commands->erase(commands->begin()+i--);
}

bool ReplaceAddWithOne::CanBeApplied(AsmCmd * line)
{
	return line->OP_ADD_Test() && line->TestRightOp("1");
}

void ReplaceAddWithOne::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->insert(commands->begin()+i++, new AsmCmd1(OP_INC, (*commands)[i]->GetLeftReg()));
	commands->erase(commands->begin()+i--);
}

bool ReplaceSubWithOne::CanBeApplied(AsmCmd * line)
{
	return line->OP_SUB_Test() && line->TestRightOp("1");
}

void ReplaceSubWithOne::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->insert(commands->begin()+i++, new AsmCmd1(OP_DEC, (*commands)[i]->GetLeftReg()));
	commands->erase(commands->begin()+i--);
}

bool ReplaceMovConst::CanBeApplied(AsmCmd * line)
{
	return line->OP_MOV_Test() && (line->TestRightOp("0") || line->TestRightOp("1") || line->TestRightOp("-1"));
}

void ReplaceMovConst::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	if ((*commands)[i]->TestRightOp("0"))
	{
		commands->insert(commands->begin()+i++, new AsmCmd2(OP_XOR, (*commands)[i]->GetLeftReg(), (*commands)[i]->GetLeftReg()));
		commands->erase(commands->begin()+i--);
	}
	else if ((*commands)[i]->TestRightOp("-1"))
	{
		commands->insert(commands->begin()+i++, new AsmCmd2(OP_OR, (*commands)[i]->GetLeftReg(), "-1"));
		commands->erase(commands->begin()+i--);
	}
	else if ((*commands)[i]->TestRightOp("1"))
	{
		commands->insert(commands->begin()+i++, new AsmCmd2(OP_XOR, (*commands)[i]->GetLeftReg(), (*commands)[i]->GetLeftReg()));
		commands->insert(commands->begin()+i++, new AsmCmd1(OP_INC, (*commands)[i]->GetLeftReg()));
		commands->erase(commands->begin()+i--);
	}
}

bool ReplaceCmpWithZero::CanBeApplied(AsmCmd * line)
{
	return line->OP_CMP_Test() && line->TestRightOp("0");
}

void ReplaceCmpWithZero::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->insert(commands->begin()+i++, new AsmCmd2(OP_TEST, (*commands)[i]->GetLeftReg(), (*commands)[i]->GetLeftReg()));
	commands->erase(commands->begin()+i--);
}

bool ReplaceMulToZero::CanBeApplied(AsmCmd * line)
{
	return line->OP_IMUL_Test() && line->TestRightOp("0");
}

void ReplaceMulToZero::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->insert(commands->begin()+i++, new AsmCmd2(OP_XOR, (*commands)[i]->GetLeftReg(), (*commands)[i]->GetLeftReg()));
	commands->erase(commands->begin()+i--);
}

bool ReplaceIMUL::CanBeApplied(AsmCmd * line)
{
	if (line->OP_IMUL_Test() && line->GetRightStringOp() != "")
	{
		int n = atoi(line->GetRightStringOp().c_str());
		switch (n)
			case 3 : case 5 : case 6 : case 9 : case 10 : case 18 : return true;
	}

	return false;
}

void ReplaceIMUL::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	int n = atoi((*commands)[i]->GetRightStringOp().c_str());
	REG reg = (*commands)[i]->GetLeftReg();
	switch (n) // Replacing some particular cases
	{
		case 3 : commands->insert(commands->begin()+i++, new AsmCmd2(OP_LEA, reg, "["+REG_STR[reg]+" + 2 * "+REG_STR[reg]+"]"));
		case 5 : commands->insert(commands->begin()+i++, new AsmCmd2(OP_LEA, reg, "["+REG_STR[reg]+" + 4 * "+REG_STR[reg]+"]"));
		case 9 : commands->insert(commands->begin()+i++, new AsmCmd2(OP_LEA, reg, "["+REG_STR[reg]+" + 8 * "+REG_STR[reg]+"]"));
		case 6 : 
			{
				commands->insert(commands->begin()+i++, new AsmCmd2(OP_LEA, reg, "["+REG_STR[reg]+" + 2 * "+REG_STR[reg]+"]"));
				commands->insert(commands->begin()+i++, new AsmCmd2(OP_ADD, reg, reg));
			}
		case 10 :
			{
				commands->insert(commands->begin()+i++, new AsmCmd2(OP_LEA, reg, "["+REG_STR[reg]+" + 4 * "+REG_STR[reg]+"]"));
				commands->insert(commands->begin()+i++, new AsmCmd2(OP_ADD, reg, reg));
			}
		case 18 :
			{
				commands->insert(commands->begin()+i++, new AsmCmd2(OP_LEA, reg, "["+REG_STR[reg]+" + 8 * "+REG_STR[reg]+"]"));
				commands->insert(commands->begin()+i++, new AsmCmd2(OP_ADD, reg, reg));
			}
	}

	commands->erase(commands->begin()+i--);
}

bool DeletePushPop::CanBeApplied(AsmCmd * lines[2])
{
	return lines[0]->OP_PUSH_Test() && lines[1]->OP_POP_Test() && lines[0]->GetLeftReg() == lines[1]->GetLeftReg()
			&& (!((lines[0]->GetLeftReg() == NOREG) && (lines[0]->GetLeftString() != lines[1]->GetLeftString())));
}

void DeletePushPop::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	commands->erase(commands->begin()+i);
	commands->erase(commands->begin()+i--); //Delete push reg/pop reg
}

bool ReplacePushPopWithMov::CanBeApplied(AsmCmd * lines[2])
{
	return lines[0]->OP_PUSH_Test() && lines[1]->OP_POP_Test() && lines[0]->GetLeftReg() != lines[1]->GetLeftReg()
									&&  (!(lines[0]->GetLeftString() != "" && lines[1]->GetLeftString() != ""));
}

void ReplacePushPopWithMov::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	string immed0 = (*commands)[i]->GetLeftString();
	string immed1 = (*commands)[i+1]->GetLeftString();
				
	//PUSH reg1/POP reg2 -> MOV reg2, reg1

	if (immed0 != "")
		commands->insert(commands->begin()+i++, new AsmCmd2(OP_MOV, (*commands)[i+1]->GetLeftReg(), immed0));
	else if (immed1 != "")
		commands->insert(commands->begin()+i++, new AsmCmd2(OP_MOV, immed1, (*commands)[i]->GetLeftReg()));
	else
		commands->insert(commands->begin()+i++, new AsmCmd2(OP_MOV, (*commands)[i+1]->GetLeftReg(), (*commands)[i]->GetLeftReg()));

	commands->erase(commands->begin()+i);
	commands->erase(commands->begin()+i--);   
}

bool ReplaceMovPushWithPush::CanBeApplied(AsmCmd * lines[2])
{
	return lines[0]->OP_MOV_Test() && lines[1]->OP_PUSH_Test() && lines[0]->GetLeftReg() == lines[1]->GetLeftReg()
												 && lines[0]->GetLeftReg() != NOREG;
}

void ReplaceMovPushWithPush::MakeOptimization(vector <AsmCmd *> * commands, size_t& i)
{
	string right_op = (*commands)[i]->GetRightStringOp();
	REG right_reg = (*commands)[i+1]->GetRightReg();

	if (right_op != "")
		commands->insert(commands->begin()+i++, new AsmCmd1(OP_PUSH, right_op));
	else if (right_reg != NOREG)
		commands->insert(commands->begin()+i++, new AsmCmd1(OP_PUSH, right_reg));

	commands->erase(commands->begin()+i);
	commands->erase(commands->begin()+i--);
}