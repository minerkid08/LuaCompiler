#include "Page.hpp"
#include "FileUtils.hpp"

void Page::writeToFile(FILE* file)
{
	for (int j = 0; j < i; j++)
	{
		fputc(buf[j], file);
	}
}

void Page::writeChar(unsigned char c)
{
	buf[i++] = c;
}

void Page::writeString(const std::string& s)
{
	for (char c : s)
		buf[i++] = c;
	buf[i++] = 0;
}

void Page::writeInt(int n)
{
	buf[i++] = (n >> (8 * 0)) & 0xff;
	buf[i++] = (n >> (8 * 1)) & 0xff;
	buf[i++] = (n >> (8 * 2)) & 0xff;
	buf[i++] = (n >> (8 * 3)) & 0xff;
}

void Page::writeToken(const Token& t, Stack<std::string>& globalVars, Stack<std::string>& localVars)
{
	if (t.type == TokenType::Text)
	{
		if (globalVars.contains(t.data))
		{
			writeChar(0);
			for (char i = 0; i < globalVars.size(); i++)
			{
				if (globalVars[i] == t.data)
				{
					writeChar(i);
					break;
				}
			}
		}
		else
		{
			writeChar(4);
			for (char i = 0; i < localVars.size(); i++)
			{
				if (localVars[i] == t.data)
				{
					writeChar(i);
					break;
				}
			}
		}
	}
	else if (t.type == TokenType::Number)
	{
		writeChar(1);
		int j = -1;
		int num = readInt(j, t.data.c_str());
		writeInt(num);
	}
	else if (t.type == TokenType::Operator)
	{
		writeChar(3);
		writeChar(t.data[0]);
	}
	else if (t.type == TokenType::Ref)
	{
		if (globalVars.contains(t.data))
		{
			writeChar(5);
			for (char i = 0; i < globalVars.size(); i++)
			{
				if (globalVars[i] == t.data)
				{
					writeChar(i);
					break;
				}
			}
		}
		else
		{
			writeChar(6);
			for (char i = 0; i < localVars.size(); i++)
			{
				if (localVars[i] == t.data)
				{
					writeChar(i);
					break;
				}
			}
		}
	}
	else if (t.type == TokenType::String)
	{
		writeChar(7);
		writeString(t.data);
	}
}
