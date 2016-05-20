#include "Generator.h"

void AsmCode::GenerateFuncAlias(string name)
{
	string alias = "procedu0";
	bool NameAlreadyExists = false;

	vector <Symbol *>::iterator it;
	map <string, string>::iterator it1;
	int i = 0;
	while (1)
	{
		for (it = vec_cpy.begin(); it != vec_cpy.end(); it++)
			if (static_cast <SymbolVar *> (*it)->name == alias)
			{
				NameAlreadyExists = true;
				break;
			}

		for (it1 = func_aliases.begin(); it1 != func_aliases.end(); it1++)
			if ((*it1).first == alias)
			{
				NameAlreadyExists = true;
				break;
			}

		if (NameAlreadyExists) 
		{
			alias.replace(alias.find(IToStr(i)), IToStr(i).length(), IToStr(i+1));
			i++;
			NameAlreadyExists = false;
			continue;
		}
		else break;
	}

	func_aliases[name] = alias;
	vec_cpy.push_back(new SymbolFunc(alias, false));
}

void AsmCode::GenerateCodeSeg(bool to_file)
{
	commands.push_back(new AsmMisc("\n.code\n\n"));
	vector <Statement *>::iterator it; //Generate code for every statement:

	map <string, vector <Statement *>>::iterator map_iter;
	for (map_iter = opers.begin(); map_iter != opers.end(); map_iter++)
	{
		if (map_iter->first != "main")
		{	
			commands.push_back(new AsmMisc(func_aliases[map_iter->first] + " proc\n"));
			commands.push_back(new AsmMisc("push ebp\nmov ebp, esp\nsub esp, " + IToStr(pars->GetFirstTable()->GetTable()[map_iter->first]->GetStackOffset()) + "\n"));			
						
			for (it = opers[map_iter->first].begin(); it != opers[map_iter->first].end(); it++)
				(*it)->AddAsm(&commands, &labels, pars->GetFirstTable(), map_iter->first, &st, &func_aliases);

			commands.push_back(new AsmMisc(func_aliases[map_iter->first] + " endp\n\n"));
		}
	}

	commands.push_back(new AsmMisc("start:\n"));
	commands.push_back(new AsmMisc("enter " + IToStr(pars->GetFirstTable()->GetTable()["main"]->GetStackOffset()) + ", 0h\n"));

	for (it = opers["main"].begin(); it != opers["main"].end(); it++)
		(*it)->AddAsm(&commands, &labels, pars->GetFirstTable(), "main", &st, &func_aliases);

	if (to_file)
	{
		commands.push_back(new AsmMisc("end start\n"));

		for (size_t i = 0; i <= commands.size() - 1; i++)
			commands[i]->GenerateToFile(output);
	}
}

void AsmCode::GenerateDataSeg()
{
	output << ".data\n\n";
	output << "aux dd 0.0 ; is used for x87 FPU" << endl;
	output << "auxq dq 0.0 ; for printf" << endl;
	output << "for_stack dd 0.0" << endl;
	output << "line_feed db 10, 13, 0" << endl;
	output << "float_format db \"%e\", 0" << endl;
	output << "str_format db \"%s\", 0" << endl;
	output << "int_format db \"%li\", 0" << endl << endl;
	output << ".data?\n\n";

	opers = pars->GetOperList();
	map <string, vector <Statement *>>::iterator map_iter;
	for (map_iter = opers.begin(); map_iter != opers.end(); map_iter++)
		if (map_iter->first != "main")
			GenerateFuncAlias(map_iter->first);

	vector <Symbol *>::iterator it;
	for (it = vec.begin(); it != vec.end(); it++)
	{
		if ((*it)->GlobalVarTest())
		{
			SymbolType * t = static_cast <SymbolVar *> (*it)->ReturnVarType();
			if (t->IntegerTest() || t->FloatTest())
				vars.push_back(new AsmUndefinedAlloc(dd, (*it)->name));

			else if (t->ArrayTest())
			{
				size_t len = static_cast <SymbolTypeArray *> (t)->ReturnFullLength();
				size_t elem= static_cast <SymbolTypeArray *> (t)->GetArrayElemSize();
				vars.push_back(new AsmArrayAlloc(dd, (*it)->name, len*elem));
			}

			else if (t->StructTest())
			{
				string name = (*it)->name;
				string type = t->name;
				vars.push_back (new AsmStrucAlloc(dd, name, type));
			}
		}
	}

	vector <AsmAllocDir *>::iterator ir;
	for (ir = vars.begin(); ir != vars.end(); ir++)
		(*ir)->GenerateToFile(output);
}

void AsmCode::GenerateConstSeg() //Here we add float constants to ".const" volume
{
	output << ".const\n" << endl;
	SymbolTable * st = pars->GetFirstTable();

	st->ReturnVarsVector(&vec);
	vec_cpy = vec;
	opers = pars->GetOperList();
	vector <Statement *>::iterator it; //Generate code for every statement
	map <string, vector <Statement *>>::iterator iter;

	for (iter = opers.begin(); iter != opers.end(); iter++)
		for (it = (*iter).second.begin(); it != (*iter).second.end(); it++)
			(*it)->GenFloatAndStringAlias(&vec_cpy, &consts);


	vector <AsmAllocDir *>::iterator ir;
	for (ir = consts.begin(); ir != consts.end(); ir++)
		(*ir)->GenerateToFile(output);

	output << "\n";
}

void AsmCode::Generate()
{
	output << "include \\masm32\\include\\masm32rt.inc\n";
	output << ".686\n";

	GenerateConstSeg();	  //Generates ".const" segment for float numbers and strings
	GenerateDataSeg();
	GenerateCodeSeg(true);
}