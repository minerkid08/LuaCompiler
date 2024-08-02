#include <sstream>
#include <vector>

#include "FileUtils.hpp"
#include "Stack.hpp"
#include "Stream.hpp"
#include "Utils.hpp"
#include "Variable.hpp"
#include "dlfcn.hpp"

Variable vars[32];
Variable localVars[32];

std::string intToHex(int i)
{
	std::stringstream s;
	s << std::hex << i;
	return s.str();
}

#define Ttype_lVar 1
#define Ttype_gVar 2
#define Ttype_lRef 3
#define Ttype_gRef 4
#define Ttype_Num 5
#define Ttype_Opr 6
#define Ttype_Expr 7
#define Ttype_Nil 8

struct Token
{
	char type;
	int data;
};

Stack<int> callStack;
Stack<int> varOffsets;
Stream<char> input;

typedef void (*NativeFunc)(const std::vector<Variable>& vars);

inline Token readToken()
{
	unsigned char type = input.consume();
	if (type == 0)
	{
		char id = input.consume();
		return {Ttype_gVar, id};
	}
	if (type == 1)
	{
		int num = readInt(input);
		return {Ttype_Num, num};
	}
	if (type == 2)
	{
		return {Ttype_Expr, 0};
	}
	if (type == 3)
	{
		char data = input.consume();
		return {Ttype_Opr, data};
	}
	if (type == 4)
	{
		char id = input.consume();
		return {Ttype_lVar, id};
	}
	if (type == 5)
	{
		char id = input.consume();
		return {Ttype_gRef, id};
	}
	if (type == 6)
	{
		char id = input.consume();
		return {Ttype_lRef, id};
	}
	return {Ttype_Nil, 0};
}

Variable parseExpr(Token v = {Ttype_Nil, 0})
{
	if (v.type == Ttype_Nil)
		v = readToken();
	if (v.type == Ttype_lRef)
	{
		return (Variable*)&(localVars[v.data + varOffsets.top()].data);
	}
	if (v.type == Ttype_gRef)
	{
		return (Variable*)&(vars[v.data].data);
	}
	if (v.type == Ttype_Num)
	{
		return v.data;
	}
	if (v.type == Ttype_gVar)
	{
		return vars[v.data].geti();
	}
	if (v.type == Ttype_lVar)
	{
		return localVars[v.data].geti();
	}
	if (v.type == Ttype_Expr)
	{
		std::vector<Variable> vars2;
		while (true)
		{
			Token v = readToken();
			if (v.type == Ttype_Opr)
				vars2.push_back({v.data, VarType::Opration});
			else
				vars2.push_back(parseExpr(v));
			if ((unsigned char)input.get(1) == 255)
			{
				input.consume();
				break;
			}
		}
		Stack<Variable> tempStack;
		int s2 = vars2.size();
		for (int i = 0; i < s2; i++)
		{
			Variable& var = vars2[i];
			if (var.type == VarType::Number)
			{
				tempStack.push(var);
			}
			else
			{
				char operation = var.geti();
				const Variable& a = tempStack[tempStack.size() - 2];
				const Variable& b = tempStack.top();
				int avalue = a.toInt();
				int bvalue = b.toInt();
				int out;
				if (operation == '+')
					out = avalue + bvalue;
				else if (operation == '-')
					out = avalue - bvalue;
				else if (operation == '*')
					out = avalue * bvalue;
				else if (operation == '/')
					out = avalue / bvalue;
				else if (operation == 1)
					out = avalue == bvalue;
				else if (operation == 2)
					out = avalue != bvalue;

				tempStack.pop(2);
				tempStack.push(out);
			}
		}
		return tempStack.top().geti();
	}
	return 0;
}

int main(int argc, const char** argv)
{
	varOffsets.push(0);

	FILE* file = fopen(argv[1], "rb");
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);
	char* input2 = new char[size + 1];
	fread(input2, 1, size, file);
	input2[size] = 0;
	fclose(file);

	input.setPtr(input2);

	input.consume(-1);

	int headerAddr = readInt(input);

	input.i = headerAddr - 1;

	std::vector<std::string> loadedLibs;
	std::vector<void*> libFunctions;
	std::vector<void*> dllData;

	while (input.get(1) != 0)
	{
		std::string name = readString(input);
		std::string dllName = name.substr(name.find(' ') + 1);
		std::string funName = name.substr(0, name.find(' '));
		if (!vecContains(loadedLibs, dllName))
		{
			dllData.push_back(dlopen((dllName + ".dll").c_str(), 0));
			loadedLibs.push_back(dllName);
			if (dllData[dllData.size() - 1] == nullptr)
				err("could not load " + dllName + ".dll");
		}
		int loadedLibInd;
		for (int i = 0; i < loadedLibs.size(); i++)
		{
			if (loadedLibs[i] == dllName)
			{
				loadedLibInd = i;
				break;
			}
		}
		input.consume();
		libFunctions.push_back(dlsym(dllData[loadedLibInd], funName.c_str()));
		if (libFunctions[libFunctions.size() - 1] == nullptr)
			err("could not load fun \'" + funName + '\'');
	}

	input.i = 3;

	while (input.get(1) != 0)
	{
		char c = input.consume();
		if (c == 1)
		{
			char type = input.consume();
			unsigned char id = input.consume();
			if (type == 2)
			{
				vars[id] = parseExpr();
			}
			else
			{
				localVars[id + varOffsets.top()] = parseExpr();
			}
		}
		else if (c == 9)
		{
			unsigned char id = input.consume();
			unsigned char argc = input.consume();
			std::vector<Variable> args;
			if (argc > 0)
			{
				for (int j = 0; j < argc; j++)
				{
					args.push_back(parseExpr());
				}
			}
			NativeFunc fun = reinterpret_cast<NativeFunc>(libFunctions[id]);
			fun(args);
		}
		else if (c == 4)
		{
			char type = input.consume();
			unsigned char id = input.consume();
			if (type == 2)
			{
				if (vars[id].type == VarType::Number)
					vars[id] = parseExpr();
				else
					*((int*)vars[id].data) = parseExpr().geti();
			}
			else if (localVars[id].type == VarType::Number)
				localVars[id] = parseExpr();
			else
				*((int*)localVars[id].data) = parseExpr().geti();
		}
		else if (c == 5)
		{
			int newI = readInt(input.i, input.getPtr());
			int e = parseExpr().toInt();
			if (e == 0)
			{
				input.i = newI - 1;
			}
		}
		else if (c == 3)
		{
			char varC = input.consume();
			int funcAddr = readInt(input);
			char argc = input.consume();
			varOffsets.push(varC);
			for (int i = 0; i < argc; i++)
			{
				unsigned char id = input.consume();
				localVars[id + varOffsets.top()] = parseExpr();
			}
			callStack.push(input.i);
			input.i = funcAddr - 1;
		}
		else if (c == 8)
		{
			input.i = callStack.top();
			callStack.pop();
			varOffsets.pop();
		}
		else if (c == 10)
		{
			int contAddr = readInt(input);
			int c = parseExpr().toInt();
			if (c == 0)
			{
				input.i = contAddr - 1;
			}
		}
		else if (c == 11)
		{
			int addr = readInt(input);
			input.i = addr - 1;
		}
		else if (c == 12)
		{
			int addr = readInt(input);
			input.i = addr - 1;
		}
	}
}
