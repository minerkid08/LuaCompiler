#include <cstdio>
#include <string>
#include <unordered_map>

#include "FileUtils.hpp"
#include "Page.hpp"
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

std::vector<std::string> libFiles;

std::vector<Page> pages = {{}};

std::unordered_map<std::string, int> funcPageIndex;

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
	if (id == 1 || id == 0 || id == 3 || id == 4 || id == 5)
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

	currentPage->name = "main";

	for (int l = 2; l < argc; l++)
	{
		if (argv[l][0] == '-' && argv[l][1] == 'l')
		{
      std::string name = argv[l];
      name = name.substr(2);
			libs[name] = {};
			getDllFunctions(name + ".dll", libs[name]);
      continue;
		}

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
			unsigned char c = input.consume();
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
					currentPage->funUssages.push_back({nativeName, currentPage->i});
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
					char varC = localVars.size();
					currentPage->writeChar(varC);
					currentPage->funUssages.push_back({name, currentPage->i});
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
					currentPage->pointers.push_back({currentPage->i, elem.pos});
				}
				else if (elem.type == LabelType::While)
				{
					vars->popTo(varInds->top());
					varInds->pop();
					int j = elem.pos;
					int currentPos = currentPage->i + 5;
					currentPage->writeChar(11);
					currentPage->pointers.push_back({j - 1, currentPage->i});
					currentPage->writeInt(0);
					currentPage->pointers.push_back({currentPage->i, j});
					for (int i = 0; i < loopLables.top().size(); i++)
					{
						int a = loopLables.top()[i];
						currentPage->pointers.push_back({currentPos, a});
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
					currentPage = &(pages[0]);
				}
				labels.pop();
			}
			else if (c == 7)
			{
				vars = &localVars;
				varInds = &localVarsInds;
				std::string name = readString(input);
				funcPageIndex[name] = pages.size();
				pages.push_back({});
				currentPage = &(pages[pages.size() - 1]);
				currentPage->name = name;
				labels.push({LabelType::Function, 4});
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
			else if (c == 10)
			{
				LabelElement elem = labels.top();
				currentPage->writeChar(13);
				vars->popTo(varInds->top());
				currentPage->pointers.push_back({currentPage->i + 4, elem.pos});
				labels.pop();
				labels.push({LabelType::If, currentPage->i});
				currentPage->writeInt(0);
			}
		}
	}

	std::unordered_map<std::string, int> funcDefs;
	std::unordered_map<std::string, std::vector<int>> funcUssages;

	Stack<int> pageQue;
	pageQue.push(0);

	while (pageQue.size() > 0)
	{
		int pageTop = ftell(file);
		Page* page = &pages[pageQue.top()];
		page->writeToFile(file);
		int ind = pageQue.top();
		pageQue.pop();
		funcDefs[page->name] = pageTop;
		for (auto& [name, refs] : page->funUssages)
		{
			if (name[name.size() - 2] == ':')
			{
				if (funcDefs.find(name) == funcDefs.end())
				{
					pageQue.push(funcPageIndex[name]);
				}
			}
			funcUssages[name].push_back(pageTop + refs);
		}
		int fend = ftell(file);
		for (Pointer& p : page->pointers)
		{
			long long pos = p.targetAddr + pageTop;
			fsetpos(file, &pos);
			writeInt(p.addr + pageTop, file);
		}
		long long pos = fend;
		fsetpos(file, &pos);
	}

	fputc(0, file);

	long long pos = ftell(file);

	std::vector<std::string> funcVec;
	for (auto& [funcName, refs] : funcUssages)
	{
		bool refFound = false;
		long long funcAddr = 0;
		if (funcDefs.find(funcName) != funcDefs.end())
		{
			funcAddr = funcDefs[funcName];
			refFound = true;
		}
		if (refFound)
		{
			for (int addr : refs)
			{
				long long addrl = addr;
				fsetpos(file, &addrl);
				fputc(funcAddr, file);
			}
			continue;
		}
		for (auto& [libName, funcs] : libs)
		{
			if (vecContains(funcs, funcName))
			{
				char id = funcIndex++;
				funcVec.push_back(funcName + " " + libName);
				for (int addr : refs)
				{
					long long addrl = addr;
					fsetpos(file, &addrl);
					fputc(id, file);
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
