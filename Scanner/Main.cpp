// Main.cpp: определяет точку входа для консольного приложения.
//
#include "HL_Optimizer.h"

void Scanning (const char *file)
{
	cout << "The result is:" << "\n\n";
	try
	{
		Scanner scanner (file);
			
		do
		{
			scanner.NextToken();
			scanner.GetToken().PrintToken(cout);
		}
		while (!scanner.CheckToken(ttEOF));
	}
	catch (SimpleError &error) {error.PrintError(cerr);}
}

void Parsing_Expr (const char *file)
{
	cout << "The result is:" << "\n\n";
	try
	{
		Scanner scanner(file);
		Parser pars(&scanner);

		while (1)
		{
			scanner.NextToken();

			if (scanner.GetTokenType() == ttEOF) break;

			if (scanner.GetTokenType() == ttInt		     ||
				scanner.GetTokenType() == ttReal		 ||
				scanner.GetTokenType() == ttIdentifier   ||
				scanner.GetTokenType() == ttPlus         ||
				scanner.GetTokenType() == ttMinus		 ||
				scanner.GetTokenType() == ttMul			 ||
				scanner.GetTokenType() == ttDiv			 ||
				scanner.GetTokenType() == ttOpenBracket)
			{
				Expr * temp = pars.ParseExpr();

				if (scanner.GetTokenType () != ttSemicolon) Error::GenParseError(code[18], scanner.line, scanner.pos);

				scanner.NextToken();
				temp->CheckLvalue(&scanner);
				temp->PrintTree(0);
				cout << "\n\n";

				if (scanner.GetTokenType() != ttEOF)
					Error::GenSimpleError("Garbage in the end of file!");
			}
			else Error::GenSimpleError("Unexpexted token at the start!");
		}
	}
	catch (SimpleError &error) {error.PrintError(cerr);}
	return;
}

void ParseOperators(const char * file)
{
	try
	{
		Scanner scanner(file);
		Parser parser(&scanner);

		scanner.NextToken();
		parser.ParseFile(0); //Start parsing

		if (!parser.MainFuncExists()) Error::GenSimpleError("There is no \"main\" function in the source code");

		parser.GetFirstTable()->PrintTable(0);		// Print the table of symbols
		parser.PrintOperators();                      // Print functions' bodies

		if (scanner.GetTokenType() != ttEOF)
					Error::GenSimpleError("Garbage in the end of file!");
	}
 	catch (SimpleError& error){error.PrintError(cerr);}

	return;
}

void GenerateCode (const char * input_file, const char * output_file, const char * opt, const char * prt_tree)
{
	try
	{
		bool ll_optimization = false;
		bool hl_optimization = false;

		Scanner scanner(input_file);
		Parser parser(&scanner);

		scanner.NextToken();
		parser.ParseFile(0); //Start parsing

		if (!parser.MainFuncExists()) Error::GenSimpleError("There is no \"main\" function in the source code");

		if (!_strnicmp(opt, "ll_opt", strlen(opt))) ll_optimization = true;
		else if (!_strnicmp(opt, "hl_opt", strlen(opt))) hl_optimization = true;
		else if (_strnicmp(opt, "no_opt", strlen(opt))) Error::GenSimpleError("Invaid optimization key");

		AsmCode * generator = new AsmCode (&parser, output_file, ll_optimization);
		Peephole * ll_opt;
		HL_Opt * hl_opt;

		if (!ll_optimization && !hl_optimization)
			generator->Generate();
		else if (ll_optimization)
		{
			ll_opt = new Peephole(generator);
			ll_opt->GenOptimizedCode();
		}
		else if (hl_optimization)
		{
			hl_opt = new HL_Opt (&parser, new Peephole(generator));
			hl_opt->Optimize();
		}

		if (!_strnicmp(prt_tree, "print_tree", strlen(prt_tree)))
			parser.PrintOperators();
	}
 	catch (SimpleError& error){error.PrintError(cerr);}
}

int main(int argc, const char *argv[])
{
	const char *s1 = argv[2];
	int len;

	if (argc > 2)
	{
		len = strlen(s1);
		if (!_strnicmp(s1, "scan", len)) Scanning(argv[1]);								 //Run scanner only
		else if (!_strnicmp(s1, "parse_expr", len)) Parsing_Expr(argv[1]);				 //Run simple expression parsing
		else if (!_strnicmp(s1, "parse_operators", len)) ParseOperators(argv[1]);        //Run full parsing
		else if (!_strnicmp(s1, "gen", len) && (argc == 6)) GenerateCode(argv[1], argv[3], argv[4], argv[5]);    //Run asm code generation
		else if (!_strnicmp(s1, "gen", len) && (argc == 5)) GenerateCode(argv[1], argv[3], argv[4], "no_print");    //Run asm code generation
	}

	else
	{
		cout << "C-subset compiler. Nikitenko Dmitry, 236" << "\n\n" ;
		cout << "The syntax is: \n\n";
		cout << "COMPILER INPUT_FILE SCAN      \n";
		cout << "COMPILER INPUT_FILE PARSE_EXPR\n";
		cout << "COMPILER INPUT_FILE PARSE_OPERATORS\n";
		cout << "COMPILER INPUT_FILE GEN OUTPUT_FILE LL_OPT\n";
		cout << "COMPILER INPUT_FILE GEN OUTPUT_FILE HL_OPT [PRINT_TREE]\n";
		cout << "COMPILER INPUT_FILE GEN OUTPUT_FILE NO_OPT\n";
	}

	cout << endl;
	//system("pause");

	return 0;
}