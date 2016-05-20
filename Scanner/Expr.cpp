#include "Statement.h"
#pragma warning (disable:4244)
#pragma warning (disable:4554)

Expr * FuncCall::FoldConst()
{
//	if (name == "printf") return this;
	for (size_t i = 0; i < arg_list.size(); i++)
		arg_list[i] = arg_list[i]->FoldConst();
	return this;
}

Expr * BinOp::FoldConst()
{
	left = left->FoldConst();
	right = right->FoldConst();

	if (value == "+" || value == "*" || value == "-" || value == "/" || value == "<<" || value == ">>" || value == "&"
					 || value == "|" || value == "^" || value == "&&" || value == "||" || value == "==" || value == "!="
					 || value == ">" || value == "<" || value == ">=" || value == "<=")
	{
		if (!((left->IntConstTest() || left->FloatConstTest()) && (right->IntConstTest() || right->FloatConstTest())))
			return this;

		else if (left->IntConstTest() && right->IntConstTest())
		{
			int v = ApplyOperator(static_cast <IntegerConstant *> (left)->GetValue(),
									static_cast <IntegerConstant *> (right)->GetValue(), 
									value);
			return new IntegerConstant(v, new SymbolTypeInteger());
		}
		else
		{
			float v1, v2;

			if (left->FloatConstTest())
				v1 = static_cast <FloatConstant *> (left)->GetValue();
			else
				v1 = static_cast <IntegerConstant *> (left)->GetValue();

			if (right->FloatConstTest())
				v2 = static_cast <FloatConstant *> (right)->GetValue();
			else
				v2 = static_cast <IntegerConstant *> (right)->GetValue();

			if (value == "+")
				return new FloatConstant(v1+v2, new SymbolTypeFloat());
			else if (value == "-")
				return new FloatConstant(v1-v2, new SymbolTypeFloat());
			else if (value == "*")
				return new FloatConstant(v1*v2, new SymbolTypeFloat());
			else if (value == "/")
				return new FloatConstant(v1/v2, new SymbolTypeFloat());
			else
				return this;
		}	
	}
	else return this;
}

Expr * UnOp::FoldConst()
{
	op = op->FoldConst();
	if (value == "-")
	{		
		if (op->IntConstTest())
		{
			int v = static_cast <IntegerConstant *> (op)->GetValue();
			return new IntegerConstant(-v, new SymbolTypeInteger());
		}
		else if (op->FloatConstTest())
		{
			float v = static_cast <FloatConstant *> (op)->GetValue();
			return new FloatConstant(-v, new SymbolTypeFloat());
		}
		else return this;
	}
	else return this;
}

Expr * BinOp::ChangeMulDivToShift()
{
	left = left->ChangeMulDivToShift();
	right = right->ChangeMulDivToShift();

	if ((value == "*" || value == "/") && left->ValueTypeInt() && right->ValueTypeInt())
	{
		int deg = 1;
		int n = static_cast <IntegerConstant *> (right)->GetValue();

		if ( (n&(n-1) != 0) || (n == 0) || (n == 1) || (n < 0) ) return this;
		else
		{
			while (n != 2)
			{
				n /= 2;
				deg++;
			}

			if (value == "*") value = "<<";
			else value = ">>";

			right = new IntegerConstant(deg, new SymbolTypeInteger());
			return this;
		}
	}
	else return this; 
}

Expr * FuncCall::ChangeMulDivToShift()
{
	if (name == "printf") return this;
	for (size_t i = 0; i < arg_list.size(); i++)
		arg_list[i] = arg_list[i]->ChangeMulDivToShift();
	return this;
}

Expr * UnOp::ChangeMulDivToShift()
{
	op = op->ChangeMulDivToShift();
	return this;
}

Expr * FuncCall::ChangeDivToMul()
{
	if (name == "printf") return this;
	for (size_t i = 0; i < arg_list.size(); i++)
		arg_list[i] = arg_list[i]->ChangeMulDivToShift();
	return this;
}

Expr * UnOp::ChangeDivToMul()
{
	op = op->ChangeDivToMul();
	return this;
}

Expr * BinOp::ChangeDivToMul()
{
	left->ChangeDivToMul();
	right->ChangeDivToMul();

	if (value == "/" && (right->ValueTypeInt() || right->ValueTypeFloat()) && ExprType->ValueTypeFloat())
	{
		float v;
		if (right->IntConstTest())
			v = static_cast <IntegerConstant *> (right)->GetValue();
		else
			v = static_cast <FloatConstant *> (right)->GetValue();

		value = "*";
		right = new FloatConstant(1/v, new SymbolTypeFloat());
	}

	return this;
}

Expr * FuncCall::DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
{
	if (name == "printf") return this;
	for (size_t i = 0; i < arg_list.size(); i++)
	{
		if (arg_list[i]->StructTest() || (const_map->top().count(arg_list[i]->ReturnName()) && value->ReturnTable()->params[i]->Formal()))
		{
			map <string, VAR_TO_DUPLICATE> tmp; //possible side-effect
			const_map->push(tmp);
		}
		arg_list[i] = arg_list[i]->DuplicateConst(const_map);
	}
	return this;
}

Expr * AssignOp::DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
{
	right = right->DuplicateConst(const_map);
	string name = left->ReturnName();

	if (left->StructTest())
		name = static_cast <Struct *> (left)->ReturnSourceName();

	if (right->IntConstTest())
	{
		const_map->top()[name].ty = INT_CONST;
		const_map->top()[name].val_i = static_cast <IntegerConstant *> (right)->GetValue();
	}
	else if (right->FloatConstTest())
	{
		const_map->top()[name].ty = FLOAT_CONST;
		const_map->top()[name].val_f = static_cast <FloatConstant *> (right)->GetValue();
	}
	
	return this;
}

Expr * Struct::DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
{
	if (const_map->top().count(ReturnSourceName()))
	{
		if (const_map->top()[ReturnSourceName()].ty == INT_CONST)
			return new IntegerConstant(const_map->top()[ReturnSourceName()].val_i, new SymbolTypeInteger());
		else
			return new FloatConstant(const_map->top()[ReturnSourceName()].val_f, new SymbolTypeFloat());
	}
	else return this;
}

Expr * Variable::DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
{
	if (const_map->top().count(name))
	{
		if (const_map->top()[name].ty == INT_CONST)
			return new IntegerConstant(const_map->top()[name].val_i, new SymbolTypeInteger());
		else
			return new FloatConstant(const_map->top()[name].val_f, new SymbolTypeFloat());
	}
	else return this;
}

Expr * BinOp::DuplicateConst(stack <map <string, VAR_TO_DUPLICATE>> * const_map)
{
	left = left->DuplicateConst(const_map);
	right = right->DuplicateConst(const_map);

	return this;
}

Expr * UnOp::DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
{
	op = op->DuplicateConst(const_map);
	return this;
}

Expr * Array::DuplicateConst(stack<map <string, VAR_TO_DUPLICATE>> * const_map)
{
	if (const_map->top().count(ReturnName()))
	{
		if (const_map->top()[ReturnName()].ty == INT_CONST)
			return new IntegerConstant(const_map->top()[ReturnName()].val_i, new SymbolTypeInteger());
		else
			return new FloatConstant(const_map->top()[ReturnName()].val_f, new SymbolTypeFloat());
	}
	else return this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Array::LocalVarTest()
{
	if (left->Var())
	{
		return static_cast <SymbolVar *> (static_cast <Variable *> (left)->GetValue())->LocalVarTest();
	}
	else return left->LocalVarTest();
}

bool Array::GlobalVarTest()
{
	if (left->Var())
	{
		return static_cast <SymbolVar *> (static_cast <Variable *> (left)->GetValue())->GlobalVarTest();
	}
	else return left->GlobalVarTest();
}

size_t Array :: ReturnBaseOffset(string var_type)
{
	if (left->Var())
	{
		if (var_type == "local")
			return static_cast <SymbolVarLocal *> (static_cast <Variable *> (left)->GetValue())->GetOffset();
		else // if param
			return static_cast <SymbolVarParam *> (static_cast <Variable *> (left)->GetValue())->GetOffset();
	}
	else return left->ReturnBaseOffset(var_type);
}

void BinOp::GenerateCode(vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	//If left and right are integers:
	if (left->ValueTypeInt() && right->ValueTypeInt() ||
		 value == "&&" || value == "||")
	{
		if (value == "<<" || value == ">>") 
		{
			left->GenerateCode(commands, EAX, labels, st, func_aliases);
			right->GenerateCode(commands, ECX, labels, st, func_aliases);
		}

		else if (value != "=")
		{
			left->GenerateCode(commands, EAX, labels, st, func_aliases);
			right->GenerateCode(commands, EBX, labels, st, func_aliases);
	
			Add (commands, new AsmCmd1(OP_POP, EBX));
			Add (commands, new AsmCmd1(OP_POP, EAX));
		}

		if (value == "+")
		{
			Add (commands, new AsmCmd2(OP_ADD, EAX, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "-")
		{
			Add (commands, new AsmCmd2(OP_SUB, EAX, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "*")
		{
			Add (commands, new AsmCmd2(OP_IMUL, EAX, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "/")
		{
			Add (commands, new AsmCmd(OP_CDQ));
			Add (commands, new AsmCmd1(OP_IDIV, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "&") 
		{
			Add (commands, new AsmCmd2(OP_AND, EAX, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "|")
		{
			Add (commands, new AsmCmd2(OP_OR, EAX, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "^")
		{
			Add (commands, new AsmCmd2(OP_XOR, EAX, EBX));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "==" || value == ">=" || value == "<=" || value == ">" || value == "<"
			  || value == "!=" || value == "&&" || value == "||")
		{
			string label1 = GenerateLabel(labels);
			string label2 = GenerateLabel(labels);
			Add (commands, new AsmCmd2(OP_CMP, EAX, EBX));

			if (value == "==")
				Add (commands, new AsmCmd1(OP_JNE, label1));
			else if (value == "!=")
				Add (commands, new AsmCmd1(OP_JE, label1));
			else if (value == ">=")
				Add (commands, new AsmCmd1(OP_JL, label1));
			else if (value == "<=")
				Add (commands, new AsmCmd1(OP_JG, label1));
			else if (value == ">")
				Add (commands, new AsmCmd1(OP_JLE, label1));
			else if (value == "<")
				Add (commands, new AsmCmd1(OP_JGE, label1));
			else if (value == "&&")
			{
				Add (commands, new AsmCmd2(OP_IMUL, EAX, EBX));
				Add (commands, new AsmCmd2(OP_CMP, EAX, IToStr(0)));
				Add (commands, new AsmCmd1(OP_JZ, label1));
			}
			else if (value == "||")
			{
				Add (commands, new AsmCmd2(OP_ADD, EAX, EBX));
				Add (commands, new AsmCmd2(OP_CMP, EAX, IToStr(0)));
				Add (commands, new AsmCmd1(OP_JZ, label1));
			}

			Add (commands, new AsmCmd2(OP_MOV, EAX, IToStr(1)));
			Add (commands, new AsmCmd1(OP_JMP, label2));
			Add (commands, new AsmLabel(label1));
			Add (commands, new AsmCmd2(OP_MOV, EAX, IToStr(0)));
			Add (commands, new AsmLabel(label2));

			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}

		else if (value == "<<" || value == ">>")
		{
			Add (commands, new AsmCmd1(OP_POP, ECX));
			Add (commands, new AsmCmd1(OP_POP, EAX));
			if (value == "<<")
				Add (commands, new AsmCmd2(OP_SHL, EAX, CL));
			else Add (commands, new AsmCmd2(OP_SHR, EAX, CL));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
			return;
		}

		else if (value == "=")
		{
			right->GenerateCode(commands, EAX, labels, st, func_aliases);
			left->GenerateAddr(commands, ECX, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, EBX));
			Add (commands, new AsmCmd1(OP_POP, EAX));
			Add (commands, new AsmCmd2 (OP_MOV, "[ebx]", EAX));			

			Add (commands, new AsmCmd1(OP_PUSH, EAX));
		}
	}

	else // if left or right is float
	{
		if (value == "+" || value == "-" || value == "*" || value == "/" || value == "&" ||
			value == "|" || value == "^")
		{
			right->GenerateCode(commands, EAX, labels, st, func_aliases);
			left->GenerateCode(commands, EAX, labels, st, func_aliases);
			
			if (value == "^" || value == "|" || value == "&")
			{
				Add (commands, new AsmCmd1(OP_POP, EAX));
				Add (commands, new AsmCmd1(OP_POP, EBX));

				if (value == "&") Add (commands, new AsmCmd2(OP_AND, EAX, EBX));
				else if (value == "|") Add (commands, new AsmCmd2(OP_OR , EAX, EBX));
				else if (value == "^") Add (commands, new AsmCmd2(OP_XOR, EAX, EBX));

				Add (commands, new AsmCmd1(OP_PUSH, EAX));

				return;
			}

			Add (commands, new AsmCmd1(OP_POP, "aux"));

			if (left->ValueTypeInt())
				Add (commands, new AsmCmd1(OP_FILD, "aux"));
			else Add (commands, new AsmCmd1(OP_FLD, "aux"));

			Add(commands, new AsmCmd1(OP_POP, "aux"));

			if (right->ValueTypeInt())
				Add(commands, new AsmCmd1(OP_FILD, "aux"));
			else Add(commands, new AsmCmd1(OP_FLD, "aux"));

			if (value == "+") Add(commands, new AsmCmd(OP_FADD));
			else if (value == "-") Add(commands, new AsmCmd(OP_FSUB));
			else if (value == "*") Add(commands, new AsmCmd(OP_FMUL));
			else if (value == "/") Add(commands, new AsmCmd(OP_FDIV));

			Add(commands, new AsmCmd1(OP_FSTP, "aux"));
			Add(commands, new AsmCmd1(OP_PUSH, "aux"));
		}

		else if (value == "=")
		{
			right->GenerateCode(commands, EAX, labels, st, func_aliases);				
			left->GenerateAddr(commands, ECX, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, EBX));
			Add (commands, new AsmCmd1(OP_POP, EAX));
			Add (commands, new AsmCmd2 (OP_MOV, "[ebx]", EAX));

			Add(commands, new AsmCmd1(OP_PUSH, EAX));
		}
		else if (value == "==" || value == "!=" || value == "<=" || value == ">=" || value == "<" || value == ">")
		{
			right->GenerateCode(commands, EAX, labels, st, func_aliases);
			left->GenerateCode(commands, EAX, labels, st, func_aliases);
			
			Add (commands, new AsmCmd1(OP_POP, "aux"));

			if (left->ValueTypeInt()) 
				Add (commands, new AsmCmd1(OP_FILD, "aux"));
			else Add (commands, new AsmCmd1(OP_FLD, "aux"));

			Add(commands, new AsmCmd1(OP_POP, "aux"));

			if (right->ValueTypeInt()) 
				Add(commands, new AsmCmd1(OP_FILD, "aux"));
			else Add(commands, new AsmCmd1(OP_FLD, "aux"));

			string label1 = GenerateLabel(labels);
			string label2 = GenerateLabel(labels);

			Add (commands, new AsmCmd2(OP_FCOMIP, ST0, ST1));
			Add (commands, new AsmCmd1(OP_FSTP, "for_stack"));

			if (value == "==")
				Add (commands, new AsmCmd1(OP_JNE, label1));
			else if (value == "!=")
				Add (commands, new AsmCmd1(OP_JE, label1));
			else if (value == "<=")
				Add (commands, new AsmCmd1(OP_JB, label1));
			else if (value == ">=")
				Add (commands, new AsmCmd1(OP_JA, label1));
			else if (value == "<")
				Add (commands, new AsmCmd1(OP_JBE, label1));
			else if (value == ">")
				Add (commands, new AsmCmd1(OP_JAE, label1));

			Add (commands, new AsmCmd2(OP_MOV, EAX, IToStr(1)));
			Add (commands, new AsmCmd1(OP_JMP, label2));
			Add (commands, new AsmLabel(label1));
			Add (commands, new AsmCmd2(OP_MOV, EAX, IToStr(0)));
			Add (commands, new AsmLabel(label2));
			Add (commands, new AsmCmd1(OP_PUSH, EAX));

			return;
		}
	}
}

void Struct::GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{	
	left->GenerateAddr(commands, reg, labels, st, func_aliases);
	right->GenerateAddr(commands, reg, labels, st, func_aliases);
	
	Add(commands, new AsmCmd1(OP_POP, EBX));
	Add(commands, new AsmCmd1(OP_POP, EAX));
	Add(commands, new AsmCmd2(OP_SUB, EAX, EBP));
	Add(commands, new AsmCmd2(OP_SUB, EBX, EBP));
	Add(commands, new AsmCmd2(OP_ADD, EAX, EBX));
	Add(commands, new AsmCmd2(OP_ADD, EAX, EBP));
	Add(commands, new AsmCmd1(OP_PUSH, EAX));
}

void Array::GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	reg = EAX;
	size_t n = 0;
	size_t elem_size = static_cast <SymbolTypeArray *> (ExprType)->GetArrayElemSize();
	stack <Expr *> expr_stack;
	SymbolTypeArray * tmp_ty = static_cast <SymbolTypeArray *> (ExprType);
	
	Array * tmp_arr = this;
	if (left->ArrayTest()) tmp_arr = static_cast <Array *> (left);

	if (left->ArrayTest())
	{
		while (tmp_arr->ArrayTest())
		{
			expr_stack.push(tmp_arr->GetRight()); // Filling the stack of indexes
			tmp_arr = static_cast <Array *> (tmp_arr->GetLeft());
		} 
	}

	if (!expr_stack.empty())
	{
		expr_stack.top()->GenerateCode(commands, reg, labels, st, func_aliases);
		Add(commands, new AsmCmd1(OP_POP, EAX));
		expr_stack.pop();
	
		n = static_cast <SymbolTypeArray *> (tmp_ty->ReturnElem_type())->GetSize();
		Add(commands, new AsmCmd2(OP_IMUL, reg, IToStr(n)));
		if (left->GlobalVarTest())
			Add(commands, new AsmCmd2(OP_LEA, ECX, "["+GetArrayName()+" + eax]")); //if global array
		else
		{
			Add(commands, new AsmCmd1(OP_NEG, EAX));
			Add(commands, new AsmCmd2(OP_LEA, ECX, "[ebp - "+IToStr(ReturnBaseOffset("local"))+" + eax]")); //Later will be not only local		
		}
	}
	else if (left->GlobalVarTest())
			Add(commands, new AsmCmd2(OP_LEA, ECX, "["+GetArrayName()+"]")); //if global array
	else
			Add(commands, new AsmCmd2(OP_LEA, ECX, "[ebp - "+IToStr(ReturnBaseOffset("local"))+"]")); //Later will be not only local

	if (static_cast <SymbolTypeArray *> (tmp_ty->ReturnElem_type())->ArrayTest())
	{
		while (!expr_stack.empty())
		{	
			tmp_ty = static_cast <SymbolTypeArray *> (tmp_ty->ReturnElem_type());			

			expr_stack.top()->GenerateCode(commands, reg, labels, st, func_aliases);
			Add(commands, new AsmCmd1(OP_POP, EAX));
			expr_stack.pop();
			n = static_cast <SymbolTypeArray *> (tmp_ty->ReturnElem_type())->GetSize();
			Add(commands, new AsmCmd2(OP_IMUL, reg, IToStr(n)));
			if (left->LocalVarTest())
				Add(commands, new AsmCmd2(OP_SUB, ECX, reg));
			else
				Add(commands, new AsmCmd2(OP_ADD, ECX, reg));
		}
	}

	right->GenerateCode(commands, reg, labels, st, func_aliases);
	Add(commands, new AsmCmd1(OP_POP, EDX));
	Add(commands, new AsmCmd2(OP_IMUL, EDX, IToStr(elem_size)));
	if (left->LocalVarTest())
		Add(commands, new AsmCmd2(OP_SUB, ECX, EDX));
	else
		Add(commands, new AsmCmd2(OP_ADD, ECX, EDX));
	Add(commands, new AsmCmd2(OP_LEA, EAX, "dword ptr [ecx]"));
	Add(commands, new AsmCmd1(OP_PUSH, EAX));
}

void IntegerConstant::GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	Add(commands, new AsmCmd2(OP_MOV, reg, IToStr(value)));
	Add(commands, new AsmCmd1(OP_PUSH, reg));
}

void FloatConstant::GenerateCode(vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	Add(commands, new AsmCmd1(OP_PUSH, ConstAlias));
}

void Variable::GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	if (value->ReturnVarType()->IntegerTest())
	{
		if (value->GlobalVarTest())
			Add(commands, new AsmCmd2(OP_MOV, reg, value->name));
		else if (value->LocalVarTest())
			Add(commands, new AsmCmd2(OP_MOV, reg, "[ebp - " + IToStr(static_cast <SymbolVarLocal *> (value)->GetOffset()) + "]"));
		else
		{
			if (static_cast <SymbolVarParam *> (value)->Formal())
			{
				Add(commands, new AsmCmd2(OP_MOV, reg, "[ebp + " + IToStr(static_cast <SymbolVarParam *> (value)->GetOffset()) + "]"));
				Add(commands, new AsmCmd2(OP_MOV, reg, "["+REG_STR[reg]+"]"));
			}
			else
				Add(commands, new AsmCmd2(OP_MOV, reg, "[ebp + " + IToStr(static_cast <SymbolVarParam *> (value)->GetOffset()) + "]"));
		}

		Add(commands, new AsmCmd1(OP_PUSH, reg));
	}

	else if (value->ReturnVarType()->FloatTest())
	{
		if (value->GlobalVarTest())
			Add(commands, new AsmCmd1(OP_PUSH, value->name));
		else if (value->LocalVarTest())
			Add(commands, new AsmCmd1(OP_PUSH, "[ebp - " + IToStr(static_cast <SymbolVarLocal *> (value)->GetOffset()) + "]"));
		else
		{
			if (static_cast <SymbolVarParam *> (value)->Formal())
			{
				Add(commands, new AsmCmd2(OP_MOV, reg, "[ebp + " + IToStr(static_cast <SymbolVarParam *> (value)->GetOffset()) + "]"));
				Add(commands, new AsmCmd2(OP_MOV, reg, "["+REG_STR[reg]+"]"));
				Add(commands, new AsmCmd1(OP_PUSH, reg));		
			}
			else Add(commands, new AsmCmd1(OP_PUSH, "[ebp + " + IToStr(static_cast <SymbolVarParam *> (value)->GetOffset()) + "]"));
		}
	}
}

void Variable::GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	if (value->GlobalVarTest())
		Add(commands, new AsmCmd2(OP_LEA, reg, "["+value->name+"]"));
	else if (value->LocalVarTest())
		Add(commands, new AsmCmd2(OP_LEA, reg, "[ebp - " + IToStr(static_cast <SymbolVarLocal *> (value)->GetOffset()) + "]"));
	else
	{
		if (static_cast <SymbolVarParam *> (value)->Formal())
			Add(commands, new AsmCmd2(OP_MOV, reg, "[ebp + " + IToStr(static_cast <SymbolVarParam *> (value)->GetOffset()) + "]"));	
		else
			Add(commands, new AsmCmd2(OP_LEA, reg, "[ebp + " + IToStr(static_cast <SymbolVarParam *> (value)->GetOffset()) + "]"));	
	}
	Add (commands, new AsmCmd1(OP_PUSH, reg));
}

void Struct::GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	GenerateAddr(commands, reg, labels, st, func_aliases);
	Add(commands, new AsmCmd1(OP_POP, EAX));
	Add(commands, new AsmCmd1(OP_PUSH, "[eax]"));
	return;
}

void UnOp::GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	if (value == "f2i" || value == "i2f")
	{
		op->GenerateCode(commands, reg, labels, st, func_aliases);

		Add(commands, new AsmCmd1(OP_POP, "aux"));
		if (value == "i2f")
			Add(commands, new AsmCmd1(OP_FILD, "aux"));
		else
			Add(commands, new AsmCmd1(OP_FLD, "aux"));

		if (value == "f2i")
			Add(commands, new AsmCmd1(OP_FISTP, "aux"));
		else 
			Add(commands, new AsmCmd1(OP_FSTP, "aux"));

		Add(commands, new AsmCmd1(OP_PUSH, "aux"));

		return;
	}

	if (op->ReturnSymbolType()->IntegerTest())
	{
		if (value == "+") op->GenerateCode(commands, reg, labels, st, func_aliases);
		if (value == "-")
		{
			op->GenerateCode (commands, reg, labels, st, func_aliases);

			Add(commands, new AsmCmd1(OP_POP, reg));
			Add(commands, new AsmCmd1(OP_NEG, reg));
			Add(commands, new AsmCmd1(OP_PUSH, reg));
		}
	}

	if (op->ReturnSymbolType()->FloatTest())
	{
		if (value == "+") op->GenerateCode(commands, reg, labels, st, func_aliases);
		if (value == "-")
		{
			op->GenerateCode(commands, reg, labels, st, func_aliases);

			Add(commands, new AsmCmd1(OP_POP, "aux"));
			Add(commands, new AsmCmd1(OP_FLD, "aux"));
			Add(commands, new AsmCmd(OP_FCHS));
			Add(commands, new AsmCmd1(OP_FSTP, "aux"));
			Add(commands, new AsmCmd1(OP_PUSH, "aux"));
		}
	}
}

void UnOp::GenerateAddr (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	op->GenerateAddr(commands, reg, labels, st, func_aliases);
	if (value == "f2i" || value == "i2f")
	{
		Add(commands, new AsmCmd1(OP_POP, reg));
		Add(commands, new AsmCmd2(OP_MOV, reg, "["+REG_STR[reg]+"]"));
		Add(commands, new AsmCmd2(OP_MOV, "aux", reg));
		if (value == "i2f")
			Add(commands, new AsmCmd1(OP_FILD, "aux"));
		else
			Add(commands, new AsmCmd1(OP_FLD, "aux"));

		if (value == "f2i")
			Add(commands, new AsmCmd1(OP_FISTP, "aux"));
		else 
			Add(commands, new AsmCmd1(OP_FSTP, "aux"));
		
		Add(commands, new AsmCmd2(OP_LEA, reg, "[aux]"));
		Add(commands, new AsmCmd1(OP_PUSH, reg));
	}
	return;
}

void FuncCall::GenerateCode (vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	vector <string> aux;

	vector <string> line_feed;
	line_feed.push_back("ADDR str_format");
	line_feed.push_back("ADDR line_feed");

	if (name == "getch")
		Add(commands, new AsmInvoke(OP_GETCH, aux));

	else if (name == "printf")
	{
		if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%i")
			aux.push_back("ADDR int_format");
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%f")
			aux.push_back("ADDR float_format");
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%s")
			aux.push_back("ADDR str_format");

		if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%i")
		{
			arg_list[1]->GenerateCode(commands, reg, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, "aux"));
			aux.push_back("aux");
		}
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%f")
		{
			aux.push_back("qword ptr auxq");
			arg_list[1]->GenerateCode(commands, reg, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, "aux"));
			Add (commands, new AsmCmd1(OP_FLD, "[aux]"));
			Add (commands, new AsmCmd1(OP_POP, "aux"));					
			Add (commands, new AsmCmd1(OP_FSTP, "auxq"));
		}
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%s")
			aux.push_back("ADDR " + static_cast <StringConstant *> (arg_list[1])->GetConstAlias());

		Add(commands, new AsmInvoke(OP_PRINTF, aux));
		Add(commands, new AsmInvoke(OP_PRINTF, line_feed));
	}	
	else
	{
		size_t params_size = 0;
		for (size_t i = 0; i < arg_list.size(); i++)
		{
			if (value->ReturnTable()->params[arg_list.size() - 1 - i]->Formal())
				params_size += 4;
			else
				params_size += arg_list[arg_list.size() - 1 - i]->GetSize();

			if (!value->ReturnTable()->params[arg_list.size() - 1 - i]->Formal())
				arg_list[arg_list.size() - 1 - i]->GenerateCode(commands, reg, labels, st, func_aliases);
			else
				arg_list[arg_list.size() - 1 - i]->GenerateAddr(commands, reg, labels, st, func_aliases);
		}
		Add (commands, new AsmCmd1(OP_CALL, (*func_aliases)[name]));
		Add (commands, new AsmCmd2(OP_ADD, ESP, IToStr(params_size)));

		if (static_cast <SymbolFunc *> (st->GetTable()[name])->lvalue())
			Add (commands, new AsmCmd1(OP_PUSH, "[eax]"));
		else
			Add (commands, new AsmCmd1(OP_PUSH, EAX));
	}
}

void FuncCall::GenerateAddr(vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	vector <string> aux;

	vector <string> line_feed;
	line_feed.push_back("ADDR str_format");
	line_feed.push_back("ADDR line_feed");

	if (name == "getch")
		Add(commands, new AsmInvoke(OP_GETCH, aux));

	else if (name == "printf")
	{
		if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%i")
			aux.push_back("ADDR int_format");
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%f")
			aux.push_back("ADDR float_format");
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%s")
			aux.push_back("ADDR str_format");

		if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%i")
		{
			arg_list[1]->GenerateCode(commands, reg, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, "aux"));
			aux.push_back("aux");
		}
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%f")
		{
			aux.push_back("qword ptr auxq");
			arg_list[1]->GenerateCode(commands, reg, labels, st, func_aliases);
			Add (commands, new AsmCmd1(OP_POP, "aux"));
			Add (commands, new AsmCmd1(OP_FLD, "[aux]"));
			Add (commands, new AsmCmd1(OP_POP, "aux"));					
			Add (commands, new AsmCmd1(OP_FSTP, "auxq"));
		}
		else if (static_cast <StringConstant *> (arg_list[0])->ReturnValue() == "%s")
			aux.push_back("ADDR " + static_cast <StringConstant *> (arg_list[1])->GetConstAlias());

		Add(commands, new AsmInvoke(OP_PRINTF, aux));
		Add(commands, new AsmInvoke(OP_PRINTF, line_feed));
	}	
	else
	{
		size_t params_size = 0;
		for (size_t i = 0; i < arg_list.size(); i++)
		{
			params_size += arg_list[arg_list.size() - 1 - i]->GetSize();
			if (!value->ReturnTable()->params[i]->Formal())
				arg_list[arg_list.size() - 1 - i]->GenerateCode(commands, reg, labels, st, func_aliases);
			else
				arg_list[arg_list.size() - 1 - i]->GenerateAddr(commands, reg, labels, st, func_aliases);
		}
		Add (commands, new AsmCmd1(OP_CALL, (*func_aliases)[name]));
		Add (commands, new AsmCmd2(OP_ADD, ESP, IToStr(params_size)));
		Add (commands, new AsmCmd1(OP_PUSH, EAX));
	}
}

string Array::GetArrayName()
{
	Expr * tmp = this;
	while (static_cast <Array *> (tmp)->GetLeft()->ArrayTest())
		tmp = static_cast <Array *> (tmp)->GetLeft();
	
	Array * tmp1 = static_cast <Array *> (tmp);
	return static_cast <Variable *> (tmp1->GetLeft())->ReturnName();
}

void Array::GenerateCode(vector <AsmCmd *> * commands, REG reg, vector <string> * labels, SymbolTable * st, map <string, string> * func_aliases)
{
	GenerateAddr(commands, reg, labels, st, func_aliases);
	Add(commands, new AsmCmd1(OP_POP, EAX));
	Add(commands, new AsmCmd1(OP_PUSH, "[eax]"));
}

void FuncCall :: GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
{
	for (size_t i = 0; i < arg_list.size(); i++)
		arg_list[i]->GenFloatAndStringAlias(vec, consts);
}

void FloatConstant::GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
{
	string alias = "fc0";
	bool NameAlreadyExists = false;

	vector <Symbol *>::iterator it;
	int i = 0;
	while (1)
	{
		for (it = vec->begin(); it != vec->end(); it++)
			if (static_cast <SymbolVar *> (*it)->name == alias)
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

	ConstAlias = alias;
	vec->push_back(new SymbolVar(ConstAlias, new SymbolTypeFloat()));
	consts->push_back(new AsmFloatAllocDir(dd, alias, value));
}

void StringConstant::GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
{
	string alias = "sc0";
	bool NameAlreadyExists = false;

	vector <Symbol *>::iterator it;
	int i = 0;
	while (1)
	{
		for (it = vec->begin(); it != vec->end(); it++)
			if (static_cast <SymbolVar *> (*it)->name == alias)
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

	ConstAlias = alias;
	vec->push_back(new SymbolVar(ConstAlias, new SymbolTypeString()));
	consts->push_back(new AsmStringAllocDir(db, alias, value));
}

string Expr::GenerateLabel(vector <string> * labels)
{
	string new_label = "L0";
	bool NameAlreadyExists = false;

	vector <string>::iterator it;
	int i = 0;
	while (1)
	{
		for (it = labels->begin(); it != labels->end(); it++)
			if ((*it) == new_label)
			{
				NameAlreadyExists = true;
				break;
			}

		if (NameAlreadyExists) 
		{
			new_label.replace(new_label.find(IToStr(i)), IToStr(i).length(), IToStr(i+1));
			i++;
			NameAlreadyExists = false;
			continue;
		}
		else break;
	}
	labels->push_back(new_label);

	return new_label;
}

void BinOp::GenFloatAndStringAlias(vector <Symbol *> * vec, vector <AsmAllocDir *> * consts)
{
	left->GenFloatAndStringAlias(vec, consts);
	right->GenFloatAndStringAlias(vec, consts);
}

bool UnOpAssign(AssignOp *t)
{
	if (t->GetRight()->UnOpTest())
	{
		if (static_cast <UnOp *> (t->GetRight())->GetOp()->Assign())
			return true;
	}
	else if (t->GetRight()->Assign()) return true;

	return false;
}

void Expr::CheckLvalue(Scanner * scn)
{
	AssignOp * t;
	if (this->Assign())
		t = static_cast <AssignOp *> (this);
	else return;

	while (UnOpAssign(t)) //here we check the situations like  a = b = 2 = c;
	{
		if (!t->GetLeft()->lvalue()) Error::GenParseError(code[0], scn->line, scn->pos);

		if (t->GetRight()->UnOpTest())
		{
			if (static_cast <UnOp *> (t->GetRight())->GetOp()->Assign())
				t = static_cast <AssignOp *> (static_cast <UnOp *> (t->GetRight())->GetOp());
		}
		else if (t->GetRight()->Assign())
			t = static_cast <AssignOp *> (t->GetRight());
	}

	if (!t->GetLeft()->lvalue()) Error::GenParseError(code[0], scn->line, scn->pos);

	return;
}

void FuncCall::PrintTree (int depth)
{
	PrintIndention(depth);
	cout << "()\n";
	PrintIndention(depth + 1);
	cout << name << endl;

	vector <Expr *> :: iterator cur; //Print arguments of calling function
	for(cur = arg_list.begin(); cur < arg_list.end(); cur++)
		(*cur)->PrintTree(depth + 2);
}

void Expr::PrintIndention (int depth)
{
	for (int i = 0; i < depth - 1; i++) cout << "    ";
	if (depth) cout << "|-- ";
}

void Struct::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << "." << endl;
	this->left->PrintTree (depth + 1);
	this->right->PrintTree (depth + 1);
}

void BinOp::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << value << endl;
	this->left->PrintTree (depth + 1);
	this->right->PrintTree (depth + 1);
}

void UnOp::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << value << endl;
	this->op->PrintTree(depth + 1);
}

void IntegerConstant::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << std::dec << value << endl;
}

void FloatConstant::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << std::fixed << value << endl; //flag is used for correct output
}

void StringConstant::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << value << endl; //flag is used for correct output
}

void Variable::PrintTree (int depth)
{
	PrintIndention (depth);
	cout << name << endl;
}

string Array::ReturnName()
{
	string full_name;
	stack <Expr *> st;

	st.push(this);
	Expr * tmp = this;
	while (static_cast <Array *> (tmp)->GetLeft()->ArrayTest())
	{
		tmp = static_cast <Array *> (tmp)->GetLeft();
		st.push(tmp);
	}
	
	Array * tmp1 = static_cast <Array *> (tmp);
	full_name += static_cast <Variable *> (tmp1->GetLeft())->ReturnName();

	while (!st.empty())
	{
		full_name += ("[" + IToStr(static_cast <IntegerConstant *> (static_cast <Array *> (tmp)->GetRight())->GetValue()) + "]");
		st.pop();
	}

	return full_name;
}

string Struct::ReturnSourceName()
{
	return left->ReturnName() + "." + right->ReturnName();
}

BinOp::BinOp(const string& token, Expr * ptr1, Expr * ptr2, SymbolTable *cur_tbll, Scanner * scn)
{
	value = token;
	string temp;

	if (value == "[]" && ptr2->ValueTypeFloat()) Error::GenParseError(code[2], scn->line, scn->pos); //Integer must be inside the square brackets
	if (ptr2->ReturnSymbolType()->VoidTest()) Error::GenParseError(code[3], scn->line, scn->pos);

	if (ptr1->ValueTypeFloat() || ptr2->ValueTypeFloat())
		ExprType = static_cast <SymbolType *> (cur_tbll->GetTable()["float"]);// Cast to float or to int
	else ExprType = static_cast <SymbolType *> (cur_tbll->GetTable()["int"]);

	if (value == "!=" || value == "==" || value == "<=" || value == ">=" || value == "<" || value == ">" ||
		value == "||" || value == "&&" || value == "&"  || value == "|"  || value == "^")
			ExprType = static_cast <SymbolType *> (cur_tbll->GetTable()["int"]);

	if (value == "<<" || value == ">>")
	{
		if (!ptr2->ValueTypeInt()) Error::GenParseError(code[1], scn->line, scn->pos); //Shift's operand must be an integer
		if (!ptr1->ValueTypeInt()) Error::GenParseError(code[1], scn->line, scn->pos);
	}		

	if (value == "=")
	{
		if (ptr1->ValueTypeFloat() && ptr2->ValueTypeInt())
		{
			left = ptr1;				   //Transforming integer to float
			right = new UnOp ("i2f", ptr2);
		}
		else if (ptr1->ValueTypeInt() && ptr2->ValueTypeFloat())
		{
			left = ptr1;
			right = new UnOp("f2i", ptr2); //Transforming float to integer, here we can lose accuracy 
			cerr << "Warning: possible loss of accuracy" << endl;
		}
		else
		{
			left = ptr1;
			right = ptr2;
		}
	}
	else
	{
		left = ptr1;
		right = ptr2;
	}
}