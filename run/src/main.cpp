#include "Variable.hpp"
#include "dlfcn.hpp"
#include <iostream>
#include <sstream>
#include <vector>

#include "FileUtils.hpp"
#include "Stack.hpp"
#include "Stream.hpp"
#include "Utils.hpp"

int vars[32];
int localVars[32];

struct Var
{
	int id;
	int value;
};

std::string intToHex(int i)
{
	std::stringstream s;
	s << std::hex << i;
	return s.str();
}

Stack<int> callStack;
Stack<int> varOffsets;
Stream<char> input;

typedef void (*NativeFunc)(const std::vector<Variable>& vars);

Var readToken()
{
	unsigned char type = input.consume();
	if (type == 0)
	{
		char id = input.consume();
		return {id, 0};
	}
	if (type == 1)
	{
		int num = readInt(input);
		return {-1, num};
	}
	if (type == 2)
	{
		return {-3, 0};
	}
	if (type == 3)
	{
		char data = input.consume();
		return {-4, data};
	}
	if (type == 4)
	{
		char id = input.consume();
		return {-5, id};
	}
	return {-2, 0};
}

int parseExpr(Var v = {-2, 0})
{
	if (v.id == -2)
		v = readToken();
	if (v.id == -1)
	{
		return v.value;
	}
	if (v.id >= 0)
	{
		return vars[v.id];
	}
	if (v.id == -5)
	{
		return localVars[v.value];
	}
	if (v.id == -3)
	{
		std::vector<Var> vars2;
		while (true)
		{
			Var v = readToken();
			if (v.id == -4)
				vars2.push_back(v);
			else
				vars2.push_back({-1, parseExpr(v)});
			if ((unsigned char)input.get(1) == 255)
			{
				input.consume();
				break;
			}
		}
		Stack<Var> tempStack;
		int s2 = vars2.size();
		for (int i = 0; i < s2; i++)
		{
			Var& var = vars2[i];
			if (var.id == -1 || var.id >= 0)
			{
				tempStack.push(var);
			}
			else
			{
				char operation = var.value;
				const Var& a = tempStack[tempStack.size() - 2];
				const Var& b = tempStack.top();
				int avalue;
				avalue = a.value;
				int bvalue;
				bvalue = b.value;
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

				Var var2;
				var2.id = -1;
				var2.value = out;
				tempStack.pop(2);
				tempStack.push(var2);
			}
		}
		return tempStack.top().value;
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
				vars[id] = parseExpr();
			else
				localVars[id + varOffsets.top()] = parseExpr();
		}
		else if (c == 5)
		{
			int newI = readInt(input.i, input.getPtr());
			if (parseExpr() == 0)
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
	}
}
