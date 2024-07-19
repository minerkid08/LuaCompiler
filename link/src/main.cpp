#include <cstdio>
#include <string>

#include "FileUtils.hpp"
#include "Stack.hpp"
#include "Token.hpp"

Stream<char> input;

Stack<int> labels;

std::unordered_map<std::string, char> vars;
std::unordered_map<std::string, char> funcs;

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
		writeToken(t, vars, file);
	}
}

int main(int argc, const char** argv)
{
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

	int varIndex = 0;
	int funcIndex = 0;

	input.consume(-1);

	while (input.get(1) != 0)
	{
		char c = input.consume();
		if (c == 1)
		{
			c = input.consume();
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input.consume();
			}
			fputc(1, file);
			fputc(varIndex, file);
			parseExpr(file);
			vars[name] = varIndex;
			varIndex++;
		}
		if (c == 2)
		{
			c = input.consume();
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input.consume();
			}
		}
		if (c == 3)
		{
			c = input.consume();
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input.consume();
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
			unsigned char argc = input.consume();
			fputc(argc, file);
			for (int j = 0; j < argc; j++)
			{
				parseExpr(file);
			}
		}
		else if (c == 4)
		{
			c = input.consume();
			std::string name;
			while (c != 0)
			{
				name += c;
				c = input.consume();
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
