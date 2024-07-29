#pragma once

#include <string>
#include "Stack.hpp"

#include "Stream.hpp"

enum class TokenType{
  Number,
  Text,
  Operator,
  Ref,
  Null
};

struct Token
{
  TokenType type;
  std::string data;
};

Token readToken(Stream<char>& t);
void writeToken(const Token& t, Stack<std::string>& globalVars, Stack<std::string>& localVars, FILE* file);
