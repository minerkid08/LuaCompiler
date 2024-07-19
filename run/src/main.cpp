#include <iostream>
#include <vector>

#include "FileUtils.hpp"
#include "Stack.hpp"
#include "Stream.hpp"

int vars[32];

struct Var
{
	int id;
	int value;
};

Stream<char> input;

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
		int num = readInt(input.i, input.getPtr());
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

	while (input.get(1) != 0)
	{
		char c = input.consume();
		if (c == 1)
		{
			unsigned char id = input.consume();
			vars[id] = parseExpr();
		}
		else if (c == 3)
		{
			unsigned char id = input.consume();
			unsigned char argc = input.consume();
			std::vector<int> args;
			for (int j = 0; j < argc; j++)
			{
				args.push_back(parseExpr());
			}
			if (id == 0)
			{
				std::cout << args[0] << "\n";
			}
		}
		else if (c == 4)
		{
			unsigned char id = input.consume();
			vars[id] = parseExpr();
		}
		else if (c == 5)
		{
			int newI = readInt(input.i, input.getPtr());
			if (parseExpr() == 0)
			{
				input.i = newI - 1;
			}
		}
	}
}
