#include <cstdio>
#include <string>
#include <unordered_map>

#include "Page.hpp"
#include "FileUtils.hpp"
#include "ReadDll.hpp"
#include "Stack.hpp"
#include "Token.hpp"
#include "Utils.hpp"

enum class LabelType
{
	If,
	While,
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

std::vector<Page> pages = {{}};

Page* currentPage = &pages[0];

int funcIndex = 0;

Stack<LabelElement> labels;
Stack<std::vector<int>> loopLables;

Stack<std::string> globalVars;
Stack<int> globalVarInds;

Stack<std::string> localVars;
Stack<int> localVarsInds;

Stack<std::string>* vars = &globalVars;
Stack<int>* varInds = &globalVarInds;

std::unordered_map<std::string, std::vector<FunctionMarker>> funcs;
std::unordered_map<std::string, std::vector<std::string>> funcArgs;
int currentMarkerIndex = 0;

void parseExpr(Stream<char>& input)
{
	char id = input.get(1);
	if (id == 2)
	{
		input.consume();
		currentPage->writeChar(2);
		unsigned char g = 0;
		while (g != 255)
		{
			parseExpr(input);
			g = input.get(1);
		}
		input.consume();
		currentPage->writeChar(255);
		return;
	}
	if (id == 1 || id == 0 || id == 3 || id == 5)
	{
		Token t = readToken(input);
		currentPage->writeToken(t, globalVars, localVars);
	}
}

void setVariable(const std::string& name, Stream<char>& input)
{
	vars->push(name);
	currentPage->writeChar(1);
	if (vars == &localVars)
		currentPage->writeChar(1);
	else
		currentPage->writeChar(2);
	currentPage->writeChar(vars->size() - 1);
	parseExpr(input);
}

void setFunVariable(const std::string& name, char id, Stream<char>& input)
{
	currentPage->writeChar(id);
	parseExpr(input);
}

int main(int argc, const char** argv)
{
	if (argc < 3)
		err("requires 2 args: out file, in file(s)");
	FILE* file = fopen(argv[1], "wb");
	currentPage->writeInt(0);

	std::unordered_map<std::string, std::vector<std::string>> libs;
	libs["stdlib"] = {};
	getDllFunctions("stdlib.dll", libs["stdlib"]);

	for (int l = 2; l < argc; l++)
	{
		Stream<char> input;
		FILE* inFile = fopen(argv[l], "rb");
		fseek(inFile, 0, SEEK_END);
		int size = ftell(inFile);
		rewind(inFile);
		char* input2 = new char[size + 1];
		fread(input2, 1, size, inFile);
		input2[size] = '\0';
		fclose(inFile);

		input.setPtr(input2);

		input.i = -1;
		while (input.get(1) != 0)
		{
			char c = input.consume();
			if (c == 1)
			{
				std::string name = readString(input);
				setVariable(name, input);
			}
			if (c == 2)
			{
				std::string name = readString(input);
				char argc = input.consume();
				if (funcArgs.find(name) == funcArgs.end())
				{
					funcArgs[name] = {};
					std::vector<std::string>* args = &(funcArgs[name]);
					for (int i = 0; i < argc; i++)
					{
						std::string arg = readString(input);
						args->push_back(arg);
					}
				}
				else
				{
					for (int i = 0; i < argc; i++)
						readString(input);
				}
			}
			if (c == 3)
			{
				std::string name = readString(input);
				std::string nativeName = name.substr(0, name.find(':'));
				bool native = false;
				for (auto& [dllname, funcList] : libs)
				{
					if (vecContains(funcList, nativeName))
					{
						native = true;
						break;
					}
				}
				if (native)
				{
					currentPage->writeChar(9);
					if (funcs.find(nativeName) == funcs.end())
						funcs[nativeName] = {};
					FunctionMarker marker = {FunctionMarkerType::Usage, currentPage->i};
					funcs[nativeName].push_back(marker);
					currentPage->writeChar(0);
					unsigned char argc = input.consume();
					currentPage->writeChar(argc);
					for (int j = 0; j < argc; j++)
					{
						parseExpr(input);
					}
				}
				else
				{
					currentPage->writeChar(3);
					if (funcs.find(name) == funcs.end())
						funcs[name] = {};
					char varC = localVars.size();
				  currentPage->writeChar(varC);
					FunctionMarker marker = {FunctionMarkerType::Usage, currentPage->i};
					funcs[name].push_back(marker);
					currentPage->writeInt(0);
					unsigned char argc = input.consume();
					currentPage->writeChar(argc);
					std::vector<std::string>* args = &(funcArgs[name]);
					for (int j = 0; j < argc; j++)
					{
						setFunVariable((*args)[j], j, input);
					}
				}
			}
			else if (c == 4)
			{
				std::string name = readString(input);
				currentPage->writeChar(4);
				for (int i = 0; i < localVars.size(); i++)
				{
					if (localVars[i] == name)
					{
						currentPage->writeChar(1);
						currentPage->writeChar(i);
						parseExpr(input);
						continue;
					}
				}
				for (int i = 0; i < globalVars.size(); i++)
				{
					if (globalVars[i] == name)
					{
						currentPage->writeChar(2);
						currentPage->writeChar(i);
						parseExpr(input);
						continue;
					}
				}
			}
			else if (c == 5)
			{
				currentPage->writeChar(5);
				varInds->push(vars->size());
				labels.push({LabelType::If, currentPage->i});
				currentPage->writeInt(currentMarkerIndex++);
				parseExpr(input);
			}
			else if (c == 6)
			{
				LabelElement elem = labels.top();
				if (elem.type == LabelType::If)
				{
					vars->popTo(varInds->top());
					varInds->pop();
					long long j = elem.pos;
					long long currentPos = currentPage->i;
					currentPage->i = j;
					currentPage->writeInt(currentPos);
					currentPage->i = currentPos;
				}
				else if (elem.type == LabelType::While)
				{
					vars->popTo(varInds->top());
					varInds->pop();
					long long j = elem.pos;
					long long currentPos = currentPage->i + 5;
					currentPage->writeChar(11);
					currentPage->writeInt(j - 1);
					currentPage->i = j;
					currentPage->writeInt(currentPos);
					for (int i = 0; i < loopLables.top().size(); i++)
					{
						long long a = loopLables.top()[i];
						currentPage->i = a;
						currentPage->writeInt(currentPos);
					}
					loopLables.pop();
					currentPage->i = currentPos;
				}
				else if (elem.type == LabelType::Function)
				{
					currentPage->writeChar(8);
					vars->popTo(0);
					varInds->popTo(0);
					vars = &globalVars;
					varInds = &globalVarInds;
				}
				labels.pop();
			}
			else if (c == 7)
			{
				vars = &localVars;
				varInds = &localVarsInds;
				std::string name = readString(input);
				if (funcs.find(name) == funcs.end())
					funcs[name] = {};
				FunctionMarker marker = {FunctionMarkerType::Definition, currentPage->i};
				labels.push({LabelType::Function, 4});
				funcs[name].push_back(marker);
				std::vector<std::string>& args = funcArgs[name];
				for (int i = 0; i < args.size(); i++)
				{
					vars->push(args[i]);
				}
			}
			else if (c == 8)
			{
				currentPage->writeChar(10);
				varInds->push(vars->size());
				labels.push({LabelType::While, currentPage->i});
				loopLables.push({});
				currentPage->writeInt(0);
				parseExpr(input);
			}
			else if (c == 9)
			{
				currentPage->writeChar(12);
				loopLables.top().push_back(currentPage->i);
				currentPage->writeInt(0);
			}
		}
	}

	currentPage->writeChar(0);
  currentPage->writeToFile(file);

	long long pos = currentPage->i;

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
