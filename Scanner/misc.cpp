#include "misc.h"

std::string IToStr (int n) 
{
 
        char * s = new char[17];
        std::string u;
 
        if (n < 0) { //turns n positive
                n = (-1 * n); 
                u = "-"; //adds '-' on result string
        }
 
        int i=0; //s counter
  
        do
		{
                s[i++]= n%10 + '0'; //conversion of each digit of n to char
                n -= n%10; //update n value
        }
 
        while ((n /= 10) > 0);
 
        for (int j = i-1; j >= 0; j--) { 
                u += s[j]; //building our string number
        }
 
        delete[] s; //free-up the memory!
        return u;
}

int ApplyOperator (int val1, int val2, string op)
{
	if (op == "+") return val1+val2;
	else if (op == "-") return val1-val2;
	else if (op == "*") return val1*val2;
	else if (op == "/") return val1/val2;
	else if (op == "<<") return val1<<val2;
	else if (op == ">>") return val1>>val2;
	else if (op == "&") return val1&val2;
	else if (op == "|") return val1|val2;
	else if (op == "^") return val1^val2;
	else if (op == "&&") return val1&&val2;
	else if (op == "||") return val1||val2;
	else if (op == "==") return val1==val2;
	else if (op == "!=") return val1!=val2;
	else if (op == "==") return val1==val2;
	else if (op == ">") return val1>val2;
	else if (op == "<") return val1<val2;
	else if (op == ">=") return val1>=val2;
	else /*if (op == "<=")*/ 
		return val1<=val2;	
}