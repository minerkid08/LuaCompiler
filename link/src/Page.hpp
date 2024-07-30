#pragma once
#include <vector>
#include <string>

#include "Token.hpp"
#include "Stack.hpp"

struct Page
{
  std::vector<std::string> funUssages;
  int i = 0;
  char buf[512];
  void writeToFile(FILE* file);
  void writeChar(unsigned char c);
  void writeInt(int i);
  void writeToken(const Token& t, Stack<std::string>& globalVars, Stack<std::string>& localVars);
};
