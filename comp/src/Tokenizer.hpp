#pragma once
#include <vector>

#include "Token.hpp"

void tokenize(const char* input, int size, std::vector<Token>* tokenPtr);
