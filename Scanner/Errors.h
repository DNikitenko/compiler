//errors.h

#include "Misc.h"

struct SimpleError
{
	string text;
	SimpleError(const string& str1) : text(str1){}
	virtual void PrintError(ostream& os)
	{
			os << "error: " << text << endl;
	}
};

struct ScannError : public SimpleError
{
		int line, pos;
		ScannError(const string& str1, int line1, int pos1) : SimpleError(str1), line(line1), pos(pos1) {}
		void PrintError(ostream& os)
		{
			os << "Scanning error (" << line << ", " << pos << ") : " << text << endl;
		}
};

struct ParseError: public SimpleError
{
		int line, pos;
		ParseError(const string& str1, int line1, int pos1) : SimpleError(str1), line(line1), pos(pos1) {}
		void PrintError(ostream& os)
		{
			os << "Parsing error (" << line << ", " << pos << ") : " << text << endl;
		}
};

struct Error
{
	static SimpleError GenSimpleError (const string& str1)
	{
		throw SimpleError(str1);
	}

	static SimpleError GenScannError (const string& str1, int line, int pos)
	{
		throw ScannError(str1, line, pos);
	}

	static SimpleError GenParseError (const string& str1, int line, int pos)
	{
		throw ParseError(str1, line, pos);
	}
};