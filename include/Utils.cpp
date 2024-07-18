#include <Utils.hpp>
#include <iostream>

void err(const std::string& msg)
{
	std::cout << msg << '\n';
	exit(1);
}

bool isLetter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isNum(char c)
{
	return c >= '0' && c <= '9';
}

bool isNum(const std::string& string)
{
	for (char c : string)
	{
		if (!isNum(c) && c != '.')
			return false;
	}
	return true;
}
