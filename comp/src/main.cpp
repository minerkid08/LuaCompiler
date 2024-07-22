#include <vector>

#include "EvalExpr.hpp"
#include "FileUtils.hpp"
#include "Stack.hpp"
#include "StreamVec.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "Utils.hpp"

#include <iostream>

FILE* file = nullptr;

StreamVec<Token> tokens;

Stack<std::string> vars;
Stack<int> blockVarPop;

std::vector<std::string> funcs;

void printToken(const Token* token)
{
	std::cout << typeToStr(token->type) << ", " << token->data << '\n';
}

void parseFunction();
void parseFunctionCall();
void parseExpr(std::vector<std::string>& endOfArgTokens);

inline bool newlineOrSemicolon(const Token* token)
{
	return *token == TokenType::NewLine || *token == TokenType::Semicolon;
}

void writeExpr(int j, const std::vector<std::vector<Token*>>& tokens)
{
	fputc(2, file);
	for (const Token* t : tokens[j])
	{
		if (t->type == TokenType::NestingHelper)
		{
			int ind = std::stoi(t->data);
			writeExpr(ind, tokens);
			continue;
		}
		writeToken(*t, file);
	}
	fputc(255, file);
}

int main(int argc, const char** argv)
{
	std::vector<Token> tokens2;
	file = fopen(argv[1], "rb");
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);
	char* input = new char[size];
	fread(input, 1, size, file);
	fclose(file);

	tokenize(input, size, &tokens2);

	delete[] input;

	for (int l = 0; l < tokens2.size(); l++)
	{
		Token* t = &(tokens2[l]);
		if (t->type == TokenType::Operator)
		{
			if (tokens2[l + 1].type == TokenType::Operator && tokens2[l + 1].data == "=")
			{
				l++;
				tokens.push({TokenType::Operator, "\1"});
				continue;
			}
		}
		tokens.push(*t);
	}

	tokens2.resize(0);

	file = fopen("out.a", "wb");

	tokens.consume(-1);

	while (*tokens.get(0) != TokenType::Eof)
	{
		const Token* token = tokens.consume();
		if (*token == TokenType::Keyword)
		{
			if (*token == "function")
			{
				parseFunction();
			}
			else if (*token == "if")
			{
				std::vector<std::string> endOfArgTokens = {"then"};
				fputc(5, file);
				parseExpr(endOfArgTokens);
				blockVarPop.push(vars.size());
			}
			else if (*token == "end")
			{
				fputc(6, file);
				int ts = blockVarPop.top();
				vars.popTo(ts);
				blockVarPop.pop();
			}
			else if (*token == "local")
			{
				const Token* token2 = tokens.consume();
				if (*token2 != TokenType::Text)
					err("variable name must be text");
				const std::string& name = token2->data;
				token2 = tokens.consume();
				if (*token2 == "=")
				{
					if (vecContains(vars.getVec(), name))
						err(std::string("var ") + name + " is already defined");
					vars.push(name);
					fputc(1, file);
					fputs(name.c_str(), file);
					fputc(0, file);
					std::vector<std::string> endOfArgTokens = {"\n", ";"};
					parseExpr(endOfArgTokens);
				}
			}
		}
		else if (*token == TokenType::Text)
		{
			const Token* token2 = tokens.get(1);
			if (*token2 == TokenType::OpenParan)
			{
				parseFunctionCall();
			}
			if (*token2 == "=")
			{
				if (vecContains(vars.getVec(), token->data))
				{
					fputc(4, file);
					fputs(token->data.c_str(), file);
					fputc(0, file);
					tokens.consume();
					std::vector<std::string> endOfArgTokens = {";", "\n"};
					parseExpr(endOfArgTokens);
				}
			}
		}
	}
	fclose(file);
	return 0;
}

void parseExpr(std::vector<std::string>& endOfArgTokens)
{
	if (vecContains(endOfArgTokens, tokens.get(2)->data))
	{
		const Token* token = tokens.consume();
		if (*token == TokenType::Text)
		{
			if (!vecContains(vars.getVec(), token->data))
			{
				err(std::string("var ") + token->data + " does not exist");
			}
		}
		writeToken(*token, file);
		tokens.consume();
	}
	else
	{
		std::vector<std::vector<Token*>> outTokens;
		int size = 0;
		int i = 0;
		int nestedLevel = 0;
		const Token* token;
		while (true)
		{
			if (vecContains(endOfArgTokens, token->data) && nestedLevel == 0)
				break;
			else
				size++;
			if (*token == TokenType::OpenParan)
				nestedLevel++;
			else if (*token == TokenType::CloseParan)
				nestedLevel--;
			token = tokens.get(size);
		}
		size--;
		for (int i2 = 0; i2 < i; i2++)
		{
			token = tokens.get(i2);
			if (*token == TokenType::Text)
			{
				if (!vecContains(vars.getVec(), token->data))
				{
					err(std::string("var ") + token->data + " does not exist");
				}
			}
		}
		parseExpression(size, &outTokens, tokens);
		writeExpr(0, outTokens);
	}
}

void parseFunctionCall()
{
	const Token* token = tokens.get();
	std::string name = token->data;
	token = tokens.consume();
	if (*token != TokenType::OpenParan)
		err("expected '(' after function name");

	int argc = 0;
	token = tokens.get();
	int j = 1;
	int nestingLevel = 0;
	if (*token != TokenType::CloseParan)
	{
		while (true)
		{
			token = tokens.get(j++);
			if (*token == TokenType::OpenParan)
				nestingLevel++;
			if (*token == TokenType::CloseParan)
			{
				if (nestingLevel == 0)
					break;
				nestingLevel--;
			}
			if (*token == TokenType::Comma)
				argc++;
		}
		argc++;
	}
	token = tokens.get(j);
	if (!newlineOrSemicolon(token))
		err("semicolon or newline expected after statement");
	name = name + ":" + std::to_string(argc);
	if (!vecContains(funcs, name))
		err(std::string("function ") + name + " with " + std::to_string(argc) + " args is not defined");
	fputc(3, file);
	fputs(name.c_str(), file);
	fputc(0, file);
	fputc(argc, file);
	std::vector<std::string> endOfArgTokens = {",", ")"};
	for (int k = 0; k < argc; k++)
	{
		parseExpr(endOfArgTokens);
	}
}

void parseFunction()
{
	const Token* token = tokens.consume();
	if (*token != TokenType::Text)
		err(std::string("function name cant be a(n) ") + typeToStr(token->type));

	std::string name = token->data;
	token = tokens.consume();
	if (*token != TokenType::OpenParan)
		err("expected '(' after function name");

	std::vector<std::string> args;
	token = tokens.consume();
	if (*token != TokenType::CloseParan)
	{
		while (true)
		{
			if (*token != TokenType::Text)
				err("function arg must be text");

			args.push_back(token->data);
			token = tokens.consume();

			if (*token == TokenType::CloseParan)
				break;
			if (*token != TokenType::Comma)
				err("function args must be seperated by commas");

			token = tokens.consume();
		}
	}
	name = name + ":" + std::to_string(args.size());
	if (!vecContains(funcs, name))
		funcs.push_back(name);
	token = tokens.consume();
	if (*token == "end")
	{
		fputc(2, file);
		writeString(name, file);
		fputc(args.size(), file);
		for (int i = 0; i < args.size(); i++)
			writeString(args[i], file);
	}
	else if (*token == "\n")
	{
		fputc(7, file);
		writeString(name, file);

		blockVarPop.push(vars.size());
	}
	else
		err("end or newline expected after function declaration");
}
