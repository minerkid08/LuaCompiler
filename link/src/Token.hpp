#pragma once

#include <string>

#include "Stream.hpp"

enum class TokenType
{
	Number,
	Text,
	Operator,
  String,
	Ref,
	Null
};

struct Token
{
	TokenType type;
	std::string data;
};

Token readToken(Stream<char>& t);
