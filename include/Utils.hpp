#pragma once
#include <string>
#include <vector>

void err(const std::string& msg);
bool isLetter(char c);
bool isNum(char c);
bool isNum(const std::string& string);
  
template <typename T> bool vecContains(const std::vector<T>& vec, const T& t)
{
	for (const T& i : vec)
	{
		if (i == t)
		{
			return true;
		}
	}
	return false;
}
