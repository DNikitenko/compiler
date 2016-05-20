#include "Statement.h"

void Statement_Return :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	vector <string> vec;

	if (fname != "main")
	{
		if (ret)
		{
			if (static_cast <SymbolFunc *> (st->GetTable()[fname])->lvalue())
				ret->GenerateAddr(commands, EAX, labels, st, func_aliases);
			else
				ret->GenerateCode(commands, EAX, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, EAX));
		}
		Add (commands, new AsmCmd2(OP_MOV, ESP, EBP));
		Add (commands, new AsmCmd1(OP_POP, EBP));
		Add (commands, new AsmCmd (OP_RET));
	}
	else
	{
		if (ret)
		{
			ret->GenerateCode(commands, EAX, labels, st, func_aliases);
			
			vec.push_back("eax");
			Add (commands, new AsmCmd(OP_LEAVE));
			Add (commands, new AsmInvoke(OP_EXITPROCESS, vec));
		}
		else
		{
			vec.push_back("0");
			Add (commands, new AsmCmd(OP_LEAVE));
			Add (commands, new AsmInvoke(OP_EXITPROCESS, vec));
		}
	}
}

void Statement_Block :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	vector <Statement *>::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++)
		(*it)->AddAsm(commands, labels, st, fname, stck, func_aliases);
}

void Statement_For :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string l1 = init->GenerateLabel(labels);
	string l2 = init->GenerateLabel(labels);

	string l3 = init->GenerateLabel(labels); // For continue
	stck->push(l3);

	stck->push(l1);
	stck->push(l2);
	stck->push("for");

	if (init) init->GenerateCode(commands, EAX, labels, st, func_aliases);
	Add (commands, new AsmLabel(l1));

	if (condition) condition->GenerateCode(commands, EAX, labels, st, func_aliases);
	else Add (commands, new AsmCmd1(OP_PUSH, "1")); //Default condition == true

	Add (commands, new AsmCmd1(OP_POP, EAX));
	Add (commands, new AsmCmd2(OP_CMP, EAX, "0"));
	Add (commands, new AsmCmd1(OP_JE, l2));

	if (body) body->AddAsm(commands, labels, st, fname, stck, func_aliases);

	Add (commands, new AsmLabel(l3)); //For continue

	if (expr_list) expr_list->GenerateCode(commands, EAX, labels, st, func_aliases);

	Add (commands, new AsmCmd1(OP_JMP, l1));
	Add (commands, new AsmLabel(l2));

	stck->pop();
	stck->pop();
	stck->pop();
	stck->pop();
}

void Statement_Do_While :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string l1 = condition->GenerateLabel(labels);
	string l2 = condition->GenerateLabel(labels);

	stck->push(l1);
	stck->push(l2);
	stck->push("do_while");

	Add (commands, new AsmLabel(l1));

	if (stmt) stmt->AddAsm(commands, labels, st, fname, stck, func_aliases);
	condition->GenerateCode(commands, EAX, labels, st, func_aliases);

	Add (commands, new AsmCmd1(OP_POP, EAX));
	Add (commands, new AsmCmd2(OP_CMP, EAX, "0"));
	Add (commands, new AsmCmd1(OP_JNE, l1));

	Add (commands, new AsmLabel(l2));

	stck->pop();
	stck->pop();
	stck->pop();
}

void Statement_While :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string l1 = condition->GenerateLabel(labels);
	string l2 = condition->GenerateLabel(labels);

	stck->push(l1);
	stck->push(l2);
	stck->push("while");

	Add (commands, new AsmLabel(l2));

	condition->GenerateCode(commands, EAX, labels, st, func_aliases);

	Add (commands, new AsmCmd1(OP_POP, EAX));
	Add (commands, new AsmCmd2(OP_CMP, EAX, IToStr(0)));
	Add (commands, new AsmCmd1(OP_JE, l1));

	if (stmt) stmt->AddAsm(commands, labels, st, fname, stck, func_aliases);

	Add (commands, new AsmCmd1(OP_JMP, l2));
	Add (commands, new AsmLabel(l1));

	stck->pop();
	stck->pop();
	stck->pop();
}

void Statement_If_Else :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string l1 = condition->GenerateLabel(labels);
	string l2 = condition->GenerateLabel(labels);

	condition->GenerateCode(commands, EAX, labels, st, func_aliases);

	Add (commands, new AsmCmd1(OP_POP, EAX));
	Add (commands, new AsmCmd2(OP_CMP, EAX, IToStr(0)));
	Add (commands, new AsmCmd1(OP_JE, l1));

	if (st1) st1->AddAsm(commands, labels, st, fname, stck, func_aliases);

	Add (commands, new AsmCmd1(OP_JMP, l2));
	Add (commands, new AsmLabel(l1));

	if (st2) st2->AddAsm(commands, labels, st, fname, stck, func_aliases);

	Add (commands, new AsmLabel(l2));
}

void Statement_If :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string l = condition->GenerateLabel(labels);

	condition->GenerateCode(commands, EAX, labels, st, func_aliases);

	Add (commands, new AsmCmd1(OP_POP, EAX));
	Add (commands, new AsmCmd2(OP_CMP, EAX, IToStr(0)));
	Add (commands, new AsmCmd1(OP_JE, l));

	if (st1) st1->AddAsm(commands, labels, st, fname, stck, func_aliases);

	Add (commands, new AsmLabel(l));
}

void Statement_Break :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string temp1 = stck->top();
	stck->pop();

	if (temp1 == "while")
	{
		string temp2 = stck->top();
		stck->pop();

		Add (commands, new AsmCmd1(OP_JMP, stck->top()));

		stck->push(temp2);
		stck->push(temp1);
	}

	else if (temp1 == "do_while"  || temp1 == "for")
	{
		Add (commands, new AsmCmd1(OP_JMP, stck->top()));
		stck->push(temp1);
	}
}

void Statement_Continue :: AddAsm (vector <AsmCmd *> * commands, vector <string> * labels, SymbolTable * st, string fname, stack <string> * stck, map <string, string> * func_aliases)
{
	string temp1 = stck->top();
	stck->pop();

	if (temp1 == "while")
	{
		Add (commands, new AsmCmd1(OP_JMP, stck->top()));
		stck->push(temp1);
	}

	else if (temp1 == "do_while")
	{
		string temp2 = stck->top();
		stck->pop();

		Add (commands, new AsmCmd1(OP_JMP, stck->top()));

		stck->push(temp2);
		stck->push(temp1);
	}
	else if (temp1 == "for")
	{
		string temp2 = stck->top();
		stck->pop();

		string temp3 = stck->top();
		stck->pop();

		Add (commands, new AsmCmd1(OP_JMP, stck->top()));

		stck->push(temp3);
		stck->push(temp2);
		stck->push(temp1);
	}
}

void Statement::PrintIndention(int depth)
{
	for (int i = 0; i < depth - 1; i++) cout << "    ";
	if (depth) cout << "|-- ";
}

void Empty_Operator::PrintStatement (int depth)
{
	PrintIndention(depth);
	cout << ";" << endl;
};

void Statement_Block::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "{}" << endl;
	vector <Statement *>::iterator iter;

	for (iter = stmts.begin(); iter != stmts.end(); iter++)
		(*iter)->PrintStatement(depth + 1);
}

void Statement_Block::CheckExprLValue(Scanner * scn)
{
	vector <Statement *>::iterator iter;

	for (iter = stmts.begin(); iter != stmts.end(); iter++)
		(*iter)->CheckExprLValue(scn);
}

void Statement_While::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "while " << endl;
	condition->PrintTree(depth + 1);

	if (stmt) stmt->PrintStatement(depth + 1);
}

void Statement_While::CheckExprLValue(Scanner * scn)
{
	condition->CheckLvalue(scn);
}

void Statement_Do_While::PrintStatement(int depth)
{
	PrintIndention(depth);

	cout << "do..while " << endl;
	stmt->PrintStatement(depth+1);
	condition->PrintTree(depth + 1);
}

void Statement_Do_While::CheckExprLValue(Scanner * scn)
{
	if (condition) condition->CheckLvalue(scn);
}

void Statement_For::CheckExprLValue(Scanner * scn)
{
	if (init) init->CheckLvalue(scn);
	if (condition) condition->CheckLvalue(scn);
	if (expr_list) expr_list->CheckLvalue(scn);
	if (body) body->CheckExprLValue(scn);
}

void Statement_For::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "for " << endl;

	if (init) init->PrintTree(depth + 1);
	else {cout << endl; PrintIndention(depth+1); cout << endl;}

	if (condition) condition->PrintTree(depth + 1);
	else {cout << endl; PrintIndention(depth+1); cout << endl;}

	if (expr_list) expr_list->PrintTree(depth + 1);
	else {cout << endl; PrintIndention(depth+1); cout << endl;}

	if (body) body->PrintStatement(depth + 1);
}

void Statement_Return::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "return " << endl;

	if (ret) ret->PrintTree(depth+1);
}

void Statement_Break::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "break " << endl;
}

void Statement_Continue::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "continue " << endl;
}

void Statement_If_Else::CheckExprLValue(Scanner * scn)
{
	condition->CheckLvalue(scn);

	if (st1) st1->CheckExprLValue(scn);
	if (st2) st2->CheckExprLValue(scn);
}

void Statement_If_Else::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "if..else " << endl;
	condition->PrintTree(depth+1);

	if (st1) st1->PrintStatement(depth+1);
	if (st2) st2->PrintStatement(depth+1);
}

void Statement_If::CheckExprLValue(Scanner * scn)
{
	condition->CheckLvalue(scn);
	if (st1) st1->CheckExprLValue(scn);
}

void Statement_If::PrintStatement(int depth)
{
	PrintIndention(depth);
	cout << "if " << endl;
	condition->PrintTree(depth+1);

	if (st1) st1->PrintStatement(depth+1);
}

void Statement_Return::CheckReturnType(SymbolTable * t, string name, Scanner * scn)
{
	if (t->GetTable().count(name))
		if (t->GetTable()[name]->FuncTest())
		{
			if (static_cast <SymbolFunc *> (t->GetTable()[name])->ReturnType()->IntegerTest() &&
				ret->ReturnSymbolType()->FloatTest())
					{
						ret = new UnOp("f2i", ret);
						cerr << "Warning: possible accuracy loss" << endl << endl;
						return;
					}
			else if (static_cast <SymbolFunc *> (t->GetTable()[name])->ReturnType()->IntegerTest() &&
				ret->ReturnSymbolType()->IntegerTest()) return;
			else if (static_cast <SymbolFunc *> (t->GetTable()[name])->ReturnType()->FloatTest() &&
				ret->ReturnSymbolType()->FloatTest()) return;
			else if (static_cast <SymbolFunc *> (t->GetTable()[name])->ReturnType()->FloatTest() &&
				ret->ReturnSymbolType()->IntegerTest())
			{
				ret = new UnOp("i2f", ret);
				return;
			}
			else if (static_cast <SymbolFunc *> (t->GetTable()[name])->ReturnType()->StructTest() &&
				ret->ReturnSymbolType()->StructTest())
			{
				if (static_cast <SymbolFunc*>(t->GetTable()[name])->ReturnType()->ReturnName() ==
					ret->ReturnSymbolType()->ReturnName())
				return;
			}

			Error::GenParseError(code[48], scn->line, scn->pos);
		}
}