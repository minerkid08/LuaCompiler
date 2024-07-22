#include <cstdio>
#include <iostream>
#include <string>

#include "FileUtils.hpp"
#include "Stack.hpp"
#include "Token.hpp"
#include "Utils.hpp"

#include "ReadDll.hpp"

#include <unordered_map>

enum class LabelType
{
	If,
	Function
};

struct LabelElement
{
	LabelType type;
	int pos;
};

enum class FunctionMarkerType
{
	Definition,
	Usage
};

struct FunctionMarker
{
	FunctionMarkerType type;
	int pos;
};

Stack<LabelElement> labels;

Stack<std::string> globalVars;
Stack<int> globalVarInds;

Stack<std::string> localVars;
Stack<int> localVarsInds;

Stack<std::string>* vars = &globalVars;
Stack<int>* varInds = &globalVarInds;

Stream<char> input;

std::unordered_map<std::string, std::vector<FunctionMarker>> funcs;
std::unordered_map<std::string, std::vector<std::string>> funcArgs;
int currentMarkerIndex = 0;

void parseExpr(FILE* file)
{
	char id = input.get(1);
	if (id == 2)
	{
		input.consume();
		fputc(2, file);
		unsigned char g = 0;
		while (g != 255)
		{
			parseExpr(file);
			g = input.get(1);
		}
		input.consume();
		fputc(255, file);
		return;
	}
	if (id == 1 || id == 0 || id == 3)
	{
		Token t = readToken(input);
		writeToken(t, globalVars, localVars, file);
	}
}

void setVariable(const std::string& name, FILE* file)
{
	vars->push(name);
	fputc(1, file);
	if (vars == &localVars)
		fputc(1, file);
	else
		fputc(2, file);
	fputc(vars->size() - 1, file);
	parseExpr(file);
}

void setFunVariable(const std::string& name, char id, FILE* file)
{
	fputc(1, file);
	fputc(1, file);
	fputc(id, file);
	parseExpr(file);
}

int main(int argc, const char** argv)
{
	std::unordered_map<std::string, std::vector<std::string>> libs;
	libs["stdlib"] = {};
	getDllFunctions("stdlib.dll", libs["stdlib"]);

	FILE* file = fopen(argv[1], "rb");
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);
	char* input2 = new char[size + 1];
	fread(input2, 1, size, file);
	input2[size] = '\0';
	fclose(file);

	input.setPtr(input2);

	file = fopen("out.o", "wb");

	int funcIndex = 0;

	input.consume(-1);
	writeInt(0, file);
	while (input.get(1) != 0)
	{
		char c = input.consume();
		if (c == 1)
		{
			std::string name = readString(input);
			setVariable(name, file);
		}
		if (c == 2)
		{
			std::string name = readString(input);
			name = name.substr(0, name.find(':'));
			char argc = input.consume();
			if (funcArgs.find(name) == funcArgs.end())
				funcArgs[name] = {};
			std::vector<std::string>* args = &(funcArgs[name]);
			for (int i = 0; i < argc; i++)
			{
				std::string arg = readString(input);
				args->push_back(arg);
			}
		}
		if (c == 3)
		{
			varInds->push(vars->size());
			std::string name = readString(input);
			name = name.substr(0, name.find(':'));
			bool native = false;
			for (auto& [dllname, funcList] : libs)
			{
				if (vecContains(funcList, name))
				{
					native = true;
					break;
				}
			}
			if (native)
			{
				putc(9, file);
				if (funcs.find(name) == funcs.end())
					funcs[name] = {};
				FunctionMarker marker = {FunctionMarkerType::Usage, ftell(file)};
				funcs[name].push_back(marker);
				fputc(0, file);
				unsigned char argc = input.consume();
				fputc(argc, file);
				for (int j = 0; j < argc; j++)
				{
					parseExpr(file);
				}
			}
			else
			{
				fputc(3, file);
				if (funcs.find(name) == funcs.end())
					funcs[name] = {};
				FunctionMarker marker = {FunctionMarkerType::Usage, ftell(file)};
				funcs[name].push_back(marker);
				writeInt(0, file);
				long long addr = ftell(file);
				writeInt(0, file);
				unsigned char argc = input.consume();
				std::vector<std::string>* args = &(funcArgs[name]);
				for (int j = 0; j < argc; j++)
				{
					setFunVariable((*args)[j], j, file);
				}
				long long top = ftell(file);
				fsetpos(file, &addr);
				writeInt(top, file);
				fsetpos(file, &top);
			}
		}
		else if (c == 4)
		{
			std::string name = readString(input);
			char ind = 0;
			for (int i = 0; i < vars->size(); i++)
			{
				if ((*vars)[i] == name)
				{
					ind = i;
				}
			}
			fputc(4, file);
			fputc(ind, file);
			parseExpr(file);
		}
		else if (c == 5)
		{
			fputc(5, file);
			labels.push({LabelType::If, ftell(file)});
			writeInt(currentMarkerIndex++, file);
			parseExpr(file);
		}
		else if (c == 6)
		{
			LabelElement elem = labels.top();
			if (elem.type == LabelType::If)
			{
				vars->popTo(varInds->top());
				varInds->pop();
				long long j = elem.pos;
				long long currentPos = ftell(file);
				fsetpos(file, &j);
				writeInt(currentPos, file);
				fsetpos(file, &currentPos);
			}
			else if (elem.type == LabelType::Function)
			{
				fputc(8, file);
				vars->popTo(0);
				varInds->popTo(0);
				vars = &globalVars;
				varInds = &globalVarInds;
			}
			labels.pop();
		}
		else if (c == 7)
		{
			fputc('i', file);
			vars = &localVars;
			varInds = &localVarsInds;
			std::string name = readString(input);
			name = name.substr(0, name.find(':'));
			if (funcs.find(name) == funcs.end())
				funcs[name] = {};
			FunctionMarker marker = {FunctionMarkerType::Definition, ftell(file)};
			labels.push({LabelType::Function, 4});
			funcs[name].push_back(marker);
			char argc = input.consume();
			std::vector<std::string> args;
			for (int i = 0; i < argc; i++)
			{
				vars->push(readString(input));
			}
		}
	}

	fputc(0, file);

	long long pos = ftell(file);

	for (auto& [funcName, refs] : funcs)
	{
		std::cout << funcName << '\n';
		for (FunctionMarker marker : refs)
		{
			std::cout << (marker.type == FunctionMarkerType::Definition ? "definition\n" : "usage\n");
		}
	}

	std::vector<std::string> funcVec;
	for (auto& [funcName, refs] : funcs)
	{
		bool refFound = false;
		long long funcAddr = 0;
		for (FunctionMarker marker : refs)
		{
			if (marker.type == FunctionMarkerType::Definition)
			{
				funcAddr = marker.pos;
				refFound = true;
				break;
			}
		}
		if (refFound)
		{
			for (FunctionMarker marker : refs)
			{
				if (marker.type == FunctionMarkerType::Usage)
				{
					long long addrl = marker.pos;
					fsetpos(file, &addrl);
					fputc(funcAddr, file);
				}
			}
			continue;
		}
		for (auto& [libName, funcs] : libs)
		{
			if (vecContains(funcs, funcName))
			{
				char id = funcIndex++;
				funcVec.push_back(funcName + " " + libName);
				for (FunctionMarker addr : refs)
				{
					if (addr.type == FunctionMarkerType::Usage)
					{
						long long addrl = addr.pos;
						fsetpos(file, &addrl);
						fputc(id, file);
					}
				}
			}
			else
				err(std::string("undefined ref to ") + funcName);
		}
	}
	long long startPos = 0;
	fsetpos(file, &startPos);
	writeInt(pos, file);
	fsetpos(file, &pos);

	char c = 0;

	for (const std::string& s : funcVec)
	{
		writeString(s, file);
		fputc(c++, file);
	}
	fputc(0, file);

	fclose(file);
}
