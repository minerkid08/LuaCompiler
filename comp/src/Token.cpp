#include "Token.hpp"
#include "FileUtils.hpp"

const char* typeToStr(TokenType type)
{
	switch (type)
	{
	case TokenType::Null:
		return "null";
	case TokenType::Operator:
		return "operator";
	case TokenType::CloseParan:
		return "closeParan";
	case TokenType::OpenParan:
		return "openParan";
	case TokenType::Text:
		return "text";
	case TokenType::Keyword:
		return "keyword";
	case TokenType::Semicolon:
		return "semicolon";
	case TokenType::String:
		return "string";
	case TokenType::Number:
		return "number";
	case TokenType::Comma:
		return "comma";
	case TokenType::NewLine:
		return "newLine";
	case TokenType::NestingHelper:
		return "nestingHelper";
	case TokenType::Eof:
		return "eof";
	}
	return "";
}

void writeToken(const Token& token, FILE* file)
{
	if (token.type == TokenType::Text)
	{
		fputc(0, file);
		writeString(token.data, file);
	}
	else if (token.type == TokenType::Number)
	{
		fputc(1, file);
		writeInt(std::stoi(token.data.c_str()), file);
	}
	else if (token.type == TokenType::Operator)
	{
		fputc(3, file);
		fputc(token.data[0], file);
	}
	else if (token.type == TokenType::String)
	{
		fputc(4, file);
		std::string name = token.data.substr(1, token.data.size() - 2);
		writeString(name, file);
	}
}

unsigned char sizeOfToken(const Token& t)
{
	if (t.type == TokenType::Text)
	{
		return 2 + t.data.size();
	}
	if (t.type == TokenType::Number)
	{
		return 5;
	}
	if (t.type == TokenType::Operator)
	{
		return 2;
	}
	return 0;
}
