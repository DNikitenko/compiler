#include <string>
#include <map>

using namespace std;

int ApplyOperator (int val1, int val2, string op);
string IToStr (int n);

const string code[] = {"Wrong left-side value", "Shift's operands must be an integer", 
					   "Float number cannot be an array index", "Void function cannot be an operand", 
					   "Array dimension mismatch", "The function contains more than 0 arguments", 
					   "Type mismatch", "Comma expected", "Closing bracket is missed", 
                       "\"main\" function cannot be called", "Invalid function call", "Unexpected token",
					   "Incorrect array", "Invalid array name", "Closing square bracket missed", 
					   "Incorrect structure", "Invalid structure name", "Identifier expected", "Semicolon expected", 
					   "Const type expected", "\"Main\" cannot be the constant", "Constant initializing expected", 
					   "Constant expected", "Wrong value type", "Opening square bracket expected", 
					   "Positive integer dimension expected", "Semicolon or comma expected", "Invalid type", 
					   "Identifier expected", "Open figure bracket expected", "Variable type expected", 
					   "Closing figure bracket expected", "Token type expected", "Wrong type","Int or float expected",
					   "incorrect type", "Operator expected", "Array type cannot be \"void\"", 
					   "Functions are not allowed in structures, blocks and function blocks", 
					   "Variable cannot be \"void\"", "Figure brackets mismatch", 
					   "Opening bracket is missed", "Opening figure bracket is missed", "\"while\" expected",
					   "Operator must be inside of any function", "\"continue\" must be inside the cycle",
					   "\"break\" must be inside the cycle", "Function must return value", "Wrong returned type"};

typedef enum {NO = -1, db = 0, dw, dd, dq, dt} MEM_ALLOC;

const string MEM_ALLOC_STR[] = {"db", "dw", "dd", "dq", "dt"};

typedef enum {NOOPCODE = -1, OP_ADD = 0, OP_SUB, OP_IMUL, OP_IDIV, OP_POP, OP_PUSH, OP_CDQ, OP_MOV,
			  OP_FLD, OP_FILD, OP_FADD, OP_FSUB, OP_FMUL, OP_FDIV, OP_FSTP, OP_NEG, OP_FCHS, OP_AND, 
			  OP_OR, OP_XOR, OP_GETCH, OP_PRINTF,
			  OP_CMP, OP_JE, OP_JNE, OP_JMP, OP_JL, OP_JG, OP_JLE, OP_JGE, OP_SHL,
			  OP_SHR, OP_CBW, OP_JZ, OP_JNZ, OP_FCOMIP, OP_JB, OP_JA, OP_JAE, OP_JBE, OP_PROC,
			  OP_ENTER, OP_ENDP, OP_LEAVE, OP_CALL, OP_RET, OP_EXITPROCESS, OP_ASSUME, OP_FISTP,
			  OP_LEA, OP_SAR, OP_INC, OP_DEC, OP_TEST} OPCODE;

const string OPCODE_STR[] = {"add", "sub", "imul", "idiv", "pop", "push", "cdq", "mov",
								  "fld", "fild", "fadd", "fsub", "fmul", "fdiv", "fstp",
								  "neg", "fchs", "and", "or", "xor",
								  "crt__getch", "crt_printf", "cmp", "je", "jne", "jmp",
								  "jl", "jg", "jle", "jge", "shl", "shr", "cbw", "jz", "jnz",
								  "fcomip", "jb", "ja", "jae", "jbe", "proc", "enter", "endp",
								  "leave", "call", "ret", "ExitProcess", "assume", "fistp", "lea",
								  "sar", "inc", "dec", "test"};

typedef enum {NOREG = -1, EAX = 0, EBX, ECX, EDX, CL, AL, CX, AX, ST0, ST1, EBP, ESP} REG;

const string REG_STR[] = {"eax", "ebx", "ecx", "edx", "cl", "al", "cx", "ax", "st(0)", "st(1)", "ebp", "esp"};

typedef enum {FLOAT_CONST = 0, INT_CONST = 1} CONST_TYPE;

struct VAR_TO_DUPLICATE	
{
	CONST_TYPE ty;
	int val_i;
	float val_f;
};
