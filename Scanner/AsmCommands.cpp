#include "AsmCommands.h"

void AsmCmd :: GenerateToFile (ofstream& os)
{
	os << OPCODE_STR[opcode] << endl;
}

void AsmCmd1 :: GenerateToFile (ofstream& os)
{
	os << OPCODE_STR[opcode] << " ";
	if (immed1 == "") os << REG_STR[operand1] << endl;
	else os << immed1 << endl;
}

void AsmCmd2 :: GenerateToFile (ofstream& os)
{
	os << OPCODE_STR[opcode] << " ";

	if (opcode == OP_ENTER)
	{
		os << immed2 << ", " << immed3 << endl;
		return;
	}

	if (immed2 == "") os << REG_STR[operand1] << ", " << REG_STR[operand2] << endl;
	else
	{
		if (operand1 != NOREG) os << REG_STR[operand1] << ", " << immed2 << endl;
		else os << immed2 << ", " << REG_STR[operand2] << endl;
	}
}

void AsmUndefinedAlloc :: GenerateToFile (ofstream& os)
{
	os << name << " " << MEM_ALLOC_STR[dir] << " ?" << endl;
}

void AsmResultPlace :: GenerateToFile (ofstream& os)
{
	os << name << endl;
}
void AsmIntAllocDir :: GenerateToFile (ofstream& os)
{
	os << name << " " << MEM_ALLOC_STR[dir] << " " << value << endl;
}

void AsmFloatAllocDir :: GenerateToFile (ofstream& os)
{
	os << name << " " << MEM_ALLOC_STR[dir] << " " << std::fixed << value << endl;
}

void AsmStringAllocDir :: GenerateToFile (ofstream& os)
{
	os << name << " " << MEM_ALLOC_STR[dir] << " \""  << value << "\"" << ", 0" << endl;
}

void AsmArrayAlloc :: GenerateToFile (ofstream& os)
{
	os << name << " dd " << size << " dup (?)" << endl;
}

void AsmInvoke :: GenerateToFile (ofstream& os)
{
	os << "invoke " << OPCODE_STR[opcode];
	vector <string>::iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++)
	{
		os << ", " << (*it);
	}
	os << endl;
}

void AsmLabel :: GenerateToFile (ofstream& os)
{
	os << label << ":" << endl;
}

void AsmProc :: GenerateToFile (ofstream& os)
{
	os << func_name << OPCODE_STR[opcode] << endl;
}

void AsmStrucAlloc ::GenerateToFile (ofstream& os)
{
	os << name << " " << type << " " << "<?>" << endl;
}

void AsmMisc::GenerateToFile(ofstream& os)
{
	os << text;
}