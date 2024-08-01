#include "Token.hpp"

#include "FileUtils.hpp"

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
