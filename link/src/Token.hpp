#pragma once

#include <string>
#include <unordered_map>

enum class TokenType{
  Number,
  Text,
  Operator,
  Null
};

struct Token
{
  TokenType type;
  std::string data;
};

Token readToken(int& i, const char* data);
void writeToken(const Token& t, std::unordered_map<std::string, char>& vars, FILE* file);
