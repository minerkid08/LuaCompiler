#include "Token.hpp"

#include "FileUtils.hpp"

#include "Stack.hpp"

Token readToken(Stream<char>& data)
{
	unsigned char type = data.consume();
	if (type == 0)
	{
		char c = data.consume();
		std::string name;
		while (c != 0)
		{
			name += c;
			c = data.consume();
		}
		return {TokenType::Text, name};
	}
	else if (type == 1)
	{
		char* c = new char[5];
		c[0] = data.consume();
		c[1] = data.consume();
		c[2] = data.consume();
		c[3] = data.consume();
		c[4] = '\0';
		return {TokenType::Number, c};
	}
	else if (type == 3)
	{
		char* c = new char[2];
		c[0] = data.consume();
		c[1] = '\0';
		return {TokenType::Operator, c};
	}
	else if (type == 5)
	{
		std::string c = readString(data);
		return {TokenType::Ref, c};
	}
	return {TokenType::Null, ""};
}

void writeToken(const Token& t, Stack<std::string>& globalVars, Stack<std::string>& localVars, FILE* file)
{
	if (t.type == TokenType::Text)
	{
		if (globalVars.contains(t.data))
		{
			fputc(0, file);
			for (char i = 0; i < globalVars.size(); i++)
			{
				if (globalVars[i] == t.data)
				{
					fputc(i, file);
					break;
				}
			}
		}
		else
		{
			fputc(4, file);
			for (char i = 0; i < localVars.size(); i++)
			{
				if (localVars[i] == t.data)
				{
					fputc(i, file);
					break;
				}
			}
		}
	}
	else if (t.type == TokenType::Number)
	{
		fputc(1, file);
		int j = -1;
		int num = readInt(j, t.data.c_str());
		writeInt(num, file);
	}
	else if (t.type == TokenType::Operator)
	{
		fputc(3, file);
		fputc(t.data[0], file);
	}
	else if (t.type == TokenType::Ref)
	{
		if (globalVars.contains(t.data))
		{
			fputc(5, file);
			for (char i = 0; i < globalVars.size(); i++)
			{
				if (globalVars[i] == t.data)
				{
					fputc(i, file);
					break;
				}
			}
		}
		else
		{
			fputc(6, file);
			for (char i = 0; i < localVars.size(); i++)
			{
				if (localVars[i] == t.data)
				{
					fputc(i, file);
					break;
				}
			}
		}
	}
}
