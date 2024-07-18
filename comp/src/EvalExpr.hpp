#pragma once

#include <vector>

#include "StreamVec.hpp"
#include "Token.hpp"

void parseExpression(int size, std::vector<std::vector<Token*>>* outTokens, StreamVec<Token>& tokens);
