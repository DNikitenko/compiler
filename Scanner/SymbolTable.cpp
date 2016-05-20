#include "SymbolTable.h"
#include <iterator>

void SymbolVarParam::PrintSymbol()
{
	cout << "param ";
	if (type->name.empty()) type->PrintSymbol();
		else if (!type->name.compare("float")) cout << "float ";
		else cout << type->name;
	if (isFormal) cout << " (formal)";
}

void SymbolVarConstInt::PrintSymbol()
{
	cout << "const ";
	type->PrintSymbol();
	cout << value;
}

void SymbolVarConstFloat::PrintSymbol()
{
	cout << "const ";
	type->PrintSymbol();
	cout << value;
}

void SymbolVarLocal::PrintSymbol()
{
	cout << "local var ";
	if (type->StructTest())
		static_cast <SymbolTypeStruct *>(type)->PrintSymbol(true);
	else type->PrintSymbol();
}

void SymbolVarGlobal::PrintSymbol()
{
	cout << "global var ";
	type->PrintSymbol();
}

void SymbolTypeArray::PrintSymbol()
{
	cout << "type arr (len " << length << ") of ";
	elem_type->PrintSymbol();
}

void SymbolTypeAlias::PrintSymbol()
{
	cout << " type ";
	ref_type->PrintSymbol();
}

SymbolType * SymbolTypeAlias::ReturnType()
{
	SymbolType *t = ref_type;
	while (t->AliasTest()) t = dynamic_cast <SymbolTypeAlias *> (t)->GetRefType();

	return t;
}

bool MemAllocDirTest(string name)
{
	return (name == "db" || name == "dw" || name == "dd" || name == "dq" || name == "dt");
}

void SymbolTable::PrintTable(int level) // Print symbol table
{
	if (level != -1) //if it wasn't called by structure
	{
		vector <SymbolTable *>::iterator it;
		for (it = vec.begin(); it < vec.end(); it++)
			(*it)->PrintTable(level + 1);

		cout << level << " level:" << endl;
	}
	map <string, Symbol *>::iterator cur;
	for (cur = table.begin(); cur != table.end(); cur++)
		if ((*cur).first.compare("int") && (*cur).first.compare("float") && (*cur).first.compare("void")) // if not int, float or void
		{
			cout << (*cur).first << ": ";
			(*cur).second->PrintSymbol();
			cout << endl;
		}
}

size_t SymbolTypeArray :: ReturnFullLength()
{
		if (elem_type->ArrayTest()) 
			return length * static_cast <SymbolTypeArray *> (elem_type)->ReturnFullLength();

		else return length;
}

SymbolType * SymbolTypeArray :: GetVarType()
{
	if (!elem_type->ArrayTest()) return elem_type;
		else return static_cast <SymbolTypeArray *> (elem_type)->GetVarType();
}

size_t SymbolFunc :: GetVarOffset(size_t var_size)
{
	current_offset += var_size;
	return (current_offset - var_size);
}

void SymbolTypeStruct :: GenStructDef(vector <string> * var_vec)
{
	map <string, Symbol *> tbl = table->GetTable();
	map <string, Symbol *>::iterator ir;

	for (ir = tbl.begin(); ir != tbl.end(); ir++)
	{
		var_vec->push_back(static_cast <SymbolVarLocal *> ((*ir).second)->name);
	}
}

void SymbolTable::ReturnVarsVector(vector <Symbol *> * consts)
{
	vector <SymbolTable *>::iterator it;
	for (it = vec.begin(); it < vec.end(); it++)
		(*it)->ReturnVarsVector(consts);

	map <string, Symbol *>::iterator cur;
	for (cur = table.begin(); cur != table.end(); cur++)
		if ((*cur).first.compare("int") && (*cur).first.compare("float") && (*cur).first.compare("void")) // if not int, float or void
			(*cur).second->PushBackVarVector(consts);																  // if variable
}

void SymbolVar::PushBackVarVector(vector <Symbol *> * consts)
{	
	vector <Symbol *>::iterator it, it1;
	for (it = consts->begin(); it != consts->end(); it++)
		if ((*it)->name == name || MemAllocDirTest(name) || (*it)->name == "c" || (*it)->name == "aux")
		{
			int i = 0;
			bool cond = false;
			do
			{
				name += IToStr(i);
 				i++;
				for (it1 = consts->begin(); it1 != consts->end(); it1++)
					if ((*it1)->name == name) cond = true;
			}
			while (cond);
		}
	if (MemAllocDirTest(name) || name == "c" || name == "aux") name += "1";
	consts->push_back(this);
}

void SymbolTypeStruct::PrintSymbol()
{
	cout << "- struct " << name << ":\n";
	cout << "{" << endl;
	table->PrintTable(-1);
	cout << "}" << endl;
}

void SymbolTypeStruct::PrintSymbol(bool foo) //if we don't want to describe inner contents of 'name'
{
	cout << "- struct " << name << "\n";
}

void SymbolFunc::PrintSymbol()
{
	if (type->VoidTest()) cout << "void func ";
	else if (type->IntegerTest()) cout << "int func ";
	else if (type->FloatTest()) cout << "float func ";
	else if (type->StructTest()) static_cast <SymbolTypeStruct *>(type)->PrintStructName();

	cout << "{" << endl;
	ftbl->PrintTable(1);
	cout << "}" << endl;
}

void SymbolTable::Add(Symbol *sym, Scanner *scn)
{
	if (table.count(sym->name) && table[sym->name]->Var())
			Error::GenParseError(("duplicate identifier \"" + sym->name + "\""), scn->line, scn->pos);
	table[sym->name] = sym;
}

Symbol * SymbolTable::GetSymbol(string name, Scanner *scn)
{   //here we try to find symbol is current and outer symbol tables
	SymbolTable * temp = this;
	if (!table.count(name))
	{
		if (this->GetAncestor() == NULL) Error::GenParseError(("unknown identifier \"" + name + "\""), scn->line, scn->pos);
		do
		{
			temp = temp->GetAncestor(); // Search higher
			if (temp->GetTable().count(name))
			{
					Symbol *sym = temp->GetTable()[name];	//Trying to find symbol on higher levels
					return sym;
			}
		}
		while (temp->GetAncestor() != NULL);
		Error::GenParseError(("unknown identifier \"" + name + "\""), scn->line, scn->pos);
	}
	Symbol *sym = table[name];
	return sym;
}

size_t SymbolTypeStruct::GetSize()
{
	map <string, Symbol *> aux = table->GetTable();
	map <string, Symbol *>::iterator it;
	size_t sum = 0;
	for (it = aux.begin(); it != aux.end(); it++)
	{
		if ((*it).first != "float" && (*it).first != "int" && (*it).first != "void" && (*it).first != "string")
			sum += (*it).second->GetSize();
	}
	return sum;
}

SymbolTable * SymbolTypeArray::ReturnTable()
{
	if (elem_type->StructTest()) return static_cast <SymbolTypeStruct *> (elem_type)->ReturnTable();
	else if (elem_type->ArrayTest()) return static_cast <SymbolTypeArray *> (elem_type)->ReturnTable();
	return NULL;
}