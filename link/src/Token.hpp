#pragma once

#include <string>
#include <unordered_map>

#include "Stream.hpp"

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

Token readToken(Stream<char>& t);
void writeToken(const Token& t, std::unordered_map<std::string, char>& vars, FILE* file);
