#include "EvalExpr.hpp"
#include "Stack.hpp"
#include "Token.hpp"

int getPriority(const Token& t)
{
	if (t.data == "+")
		return 2;
	if (t.data == "-")
		return 1;
	if (t.data == "*")
		return 4;
	if (t.data == "/")
		return 3;
	if (t.data == "\1")
		return 0;
	if (t.data == "\2")
		return 0;
	if (t.data == "\3")
		return 99;
	return -1;
}

void parseExpression(int size, std::vector<std::vector<Token*>>* parsedTokenMap, StreamVec<Token>& tokens)
{
	std::vector<std::vector<Token*>> nestingMap;

	Stack<int> nestingStack;

	nestingMap.push_back({});
	std::vector<Token*>* currentVec = &nestingMap[0];

	int mapIndex = 0;

	for (int i = 0; i < size; i++)
	{
		const Token* token = tokens.consume();
		if (*token == TokenType::OpenParan)
		{
			nestingStack.push(mapIndex);
			mapIndex = nestingMap.size();
			Token* t = new Token();
			t->type = TokenType::NestingHelper;
			t->data = std::to_string(mapIndex);
			currentVec->push_back(t);
			nestingMap.push_back({});
			currentVec = &nestingMap[mapIndex];
		}
		if (*token == TokenType::OpenSqBk)
		{
			Token* t = new Token();
			t->type = TokenType::Operator;
      t->data = "\3";
			currentVec->push_back(t);
			nestingStack.push(mapIndex);
			mapIndex = nestingMap.size();
			t = new Token();
			t->type = TokenType::NestingHelper;
			t->data = std::to_string(mapIndex);
			currentVec->push_back(t);
			nestingMap.push_back({});
			currentVec = &nestingMap[mapIndex];
		}
		else if (*token == TokenType::CloseParan || *token == TokenType::CloseSqBk)
		{
			mapIndex = nestingStack.top();
			nestingStack.pop(1);
			currentVec = &nestingMap[mapIndex];
		}
		else
			currentVec->push_back((Token*)token);
	}

	for (const std::vector<Token*>& tokenList : nestingMap)
	{
		Stack<Token*> outputStack;
		Stack<Token*> tempStack;
		for (Token* token : tokenList)
		{
			if (*token == TokenType::Number || *token == TokenType::Text || *token == TokenType::NestingHelper ||
				*token == TokenType::String)
			{
				outputStack.push(token);
			}
			else if (*token == TokenType::Operator)
			{
				int priority = getPriority(*token);
				bool pushed = false;
				while (!pushed)
				{
					if (tempStack.size() > 0)
					{
						if (getPriority(*tempStack.top()) < priority)
						{
							tempStack.push(token);
							pushed = true;
						}
						else
						{
							outputStack.push(tempStack.top());
							tempStack.pop(1);
						}
					}
					else
					{
						tempStack.push(token);
						pushed = true;
					}
				}
			}
		}
		int s2 = tempStack.size();
		for (int j = 0; j < s2; j++)
		{
			outputStack.push(tempStack.top());
			tempStack.pop(1);
		}

		s2 = outputStack.size();
		parsedTokenMap->push_back({});
		std::vector<Token*>& currentVec = (*parsedTokenMap)[parsedTokenMap->size() - 1];
		for (int i = 0; i < s2; i++)
		{
			currentVec.push_back(outputStack[i]);
		}
	}
}
