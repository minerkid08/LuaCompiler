#pragma once
#include <string>

enum class TokenType
{
	Null,
	Text,
	OpenParan,
	CloseParan,
	Keyword,
	Number,
	String,
	Semicolon,
	Comma,
	NewLine,
	Operator,
	NestingHelper,
	Eof
};

struct Token
{
	TokenType type;
	std::string data;
	bool operator==(TokenType other) const
	{
		return type == other;
	}
	bool operator==(const Token& other) const
	{
		return type == other.type && data == other.data;
	}
	bool operator==(const std::string& other) const
	{
		return data == other;
	}
	bool operator!=(TokenType other) const
	{
		return type != other;
	}
	bool operator!=(const Token& other) const
	{
		return type != other.type || data != other.data;
	}
	bool operator!=(const std::string& other) const
	{
		return data != other;
	}
};

const char* typeToStr(TokenType type);

void writeToken(const Token& t, FILE* file);
Token readToken(int& i, const char* data);
unsigned char sizeOfToken(const Token& t);
