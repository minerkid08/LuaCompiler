#pragma once
#include <string>
#include <vector>

#include "Stack.hpp"
#include "Token.hpp"

struct Function
{
	std::string name;
	int pos;
};

struct Page
{
  std::string name;
	std::vector<Function> funUssages;
	int i = 0;
	char buf[512];
	void writeToFile(FILE* file);
	void writeChar(unsigned char c);
	void writeInt(int i);
	void writeToken(const Token& t, Stack<std::string>& globalVars, Stack<std::string>& localVars);
};
