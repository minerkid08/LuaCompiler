#include <cstdio>
#include <string>
#include <unordered_map>

#include "FileUtils.hpp"
#include "ReadDll.hpp"
#include "Stack.hpp"
#include "Token.hpp"
#include "Utils.hpp"

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

FILE* file;

int funcIndex = 0;

Stack<LabelElement> labels;

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
		fputc(2, file);
		unsigned char g = 0;
		while (g != 255)
		{
			parseExpr(input);
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

void setVariable(const std::string& name, Stream<char>& input)
{
	vars->push(name);
	fputc(1, file);
	if (vars == &localVars)
		fputc(1, file);
	else
		fputc(2, file);
	fputc(vars->size() - 1, file);
	parseExpr(input);
}

void setFunVariable(const std::string& name, char id, Stream<char>& input)
{
	fputc(id, file);
	parseExpr(input);
}

int main(int argc, const char** argv)
{
	if (argc < 3)
		err("requires 2 args: out file, in file(s)");
	file = fopen(argv[1], "wb");
	writeInt(0, file);

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
				varInds->push(vars->size());
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
					putc(9, file);
					if (funcs.find(nativeName) == funcs.end())
						funcs[nativeName] = {};
					FunctionMarker marker = {FunctionMarkerType::Usage, ftell(file)};
					funcs[nativeName].push_back(marker);
					fputc(0, file);
					unsigned char argc = input.consume();
					fputc(argc, file);
					for (int j = 0; j < argc; j++)
					{
						if (input.get(1) == 5)
						{
              input.consume();
							std::string str = readString(input);
							if (globalVars.contains(str))
							{
								fputc(6, file);
								for (char i = 0; i < globalVars.size(); i++)
								{
									if (globalVars[i] == str)
									{
										fputc(i, file);
										break;
									}
								}
							}
							else
							{
								fputc(5, file);
								for (char i = 0; i < localVars.size(); i++)
								{
									if (localVars[i] == str)
									{
										fputc(i, file);
										break;
									}
								}
							}
						}
						else
							parseExpr(input);
					}
				}
				else
				{
					fputc(3, file);
					if (funcs.find(name) == funcs.end())
						funcs[name] = {};
					char varC = localVars.size();
					fputc(varC, file);
					FunctionMarker marker = {FunctionMarkerType::Usage, ftell(file)};
					funcs[name].push_back(marker);
					writeInt(0, file);
					unsigned char argc = input.consume();
					fputc(argc, file);
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
				fputc(4, file);
				for (int i = 0; i < localVars.size(); i++)
				{
					if (localVars[i] == name)
					{
						fputc(1, file);
						fputc(i, file);
						parseExpr(input);
						continue;
					}
				}
				for (int i = 0; i < globalVars.size(); i++)
				{
					if (globalVars[i] == name)
					{
						fputc(2, file);
						fputc(i, file);
						parseExpr(input);
						continue;
					}
				}
			}
			else if (c == 5)
			{
				fputc(5, file);
				labels.push({LabelType::If, ftell(file)});
				writeInt(currentMarkerIndex++, file);
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
				vars = &localVars;
				varInds = &localVarsInds;
				std::string name = readString(input);
				if (funcs.find(name) == funcs.end())
					funcs[name] = {};
				FunctionMarker marker = {FunctionMarkerType::Definition, ftell(file)};
				labels.push({LabelType::Function, 4});
				funcs[name].push_back(marker);
				std::vector<std::string>& args = funcArgs[name];
				for (int i = 0; i < args.size(); i++)
				{
					vars->push(args[i]);
				}
			}
		}
	}

	fputc(0, file);

	long long pos = ftell(file);

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
