#include <cstdio>
#include <string>

#include "FileUtils.hpp"
#include "Stack.hpp"
#include "Token.hpp"

char* input;

int i;

Stack<int> labels;

std::unordered_map<std::string, char> vars;
std::unordered_map<std::string, char> funcs;

int currentMarkerIndex = 0;

void parseExpr(FILE* file)
{
	if (input[i + 1] == 2)
	{
		i++;
		fputc(2, file);
		unsigned char g = 0;
		while (g != 255)
		{
			parseExpr(file);
			g = input[i + 1];
		}
		i++;
		fputc(255, file);
		return;
	}
	if (input[i + 1] == 1 || input[i + 1] == 0 || input[i + 1] == 3)
	{
		Token t = readToken(i, input);
		writeToken(t, vars, file);
	}
}

int main(int argc, const char** argv)
{
	FILE* file = fopen(argv[1], "rb");
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);
	input = new char[size];
	fread(input, 1, size, file);
	fclose(file);

	file = fopen("out.o", "wb");

	int varIndex = 0;
	int funcIndex = 0;

	i = -1;
	while (i < size)
	{
		char c = input[++i];
		if (c == 1)
		{
			c = input[++i];
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input[++i];
			}
			fputc(1, file);
			fputc(varIndex, file);
			parseExpr(file);
			vars[name] = varIndex;
			varIndex++;
		}
		if (c == 3)
		{
			c = input[++i];
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input[++i];
			}
			fputc(3, file);
			int id;
			if (funcs.find(name) != funcs.end())
				id = funcs[name];
			else
			{
				funcs[name] = funcIndex;
				id = funcIndex++;
			}
			fputc(id, file);
			unsigned char argc = input[++i];
			fputc(argc, file);
			int j = 0;
			while (j < argc)
			{
				parseExpr(file);
				j++;
			}
		}
		else if (c == 4)
		{
			c = input[++i];
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input[++i];
			}
			int ind = vars[name];
			fputc(4, file);
			fputc(ind, file);
			parseExpr(file);
		}
		else if (c == 5)
		{
			fputc(5, file);
			labels.push(ftell(file));
			writeInt(currentMarkerIndex++, file);
			parseExpr(file);
		}
		else if (c == 6)
		{
			long long j = labels.top();
			long long currentPos = ftell(file);
			fsetpos(file, &j);
			writeInt(currentPos, file);
			fsetpos(file, &currentPos);
		}
	}
	fclose(file);
}
