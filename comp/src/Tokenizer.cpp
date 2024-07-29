#include <Tokenizer.hpp>
#include <Utils.hpp>

static std::vector<Token>* tokens = nullptr;

std::vector<std::string> keywords = {
  "function",
  "end",
  "if",
  "then",
  "do",
  "local",
  "while",
  "break",
  "require",
  "ref"
};

std::vector<std::string> operators = {
  "+", "-", "*", "/", "=", "!"
};

void addToken(std::string& str)
{
	char* token = new char[str.size() + 1];
	token[str.size()] = '\0';
	memcpy(token, str.c_str(), str.size());
	TokenType type;
	if (str == ")")
		type = TokenType::CloseParan;
	else if (str == "(")
		type = TokenType::OpenParan;
	else if (vecContains(keywords, str))
		type = TokenType::Keyword;
	else if (isNum(str))
		type = TokenType::Number;
	else if (str[0] == '\"')
		type = TokenType::String;
	else if (str == ";")
		type = TokenType::Semicolon;
	else if (str == ",")
		type = TokenType::Comma;
	else if (str == "\n")
		type = TokenType::NewLine;
	else if (vecContains(operators, str))
		type = TokenType::Operator;
	else
		type = TokenType::Text;
	tokens->push_back(Token{type, token});
	str = "";
}

void tokenize(const char* input, int size, std::vector<Token>* tokenPtr)
{
	tokens = tokenPtr;
	std::string tokenStr;
	bool inToken = false;
	bool inStr = false;
	for (int i = 0; i < size; i++)
	{
		char c = input[i];
		if (!inStr)
		{
			if (inToken)
			{
				if (!isNum(c) && !isLetter(c))
				{
					addToken(tokenStr);
					inToken = false;
				}
				else
				{
					tokenStr += c;
				}
			}
			if (c != '\t' && c != '\r' && c != ' ' && !inToken)
			{
				inToken = true;
				tokenStr += c;
				if (!isNum(c) && !isLetter(c))
				{
					if (c == '\"')
					{
						inStr = true;
					}
					else
					{
						addToken(tokenStr);
						inToken = false;
					}
				}
			}
		}
		else
		{
			tokenStr += c;
			if (c == '\"')
			{
				inStr = false;
				inToken = false;
				addToken(tokenStr);
			}
		}
	}
  tokens->push_back({TokenType::Eof, "eof"});
}
