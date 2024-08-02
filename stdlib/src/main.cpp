#include <iostream>
#include <vector>

#include "Variable.hpp"

#define EXPORT __declspec(dllexport)

extern "C"
{
	void EXPORT print(const std::vector<Variable>& vars)
	{
		std::cout << vars[0].toInt();
	}
	void EXPORT println(const std::vector<Variable>& vars)
	{
		std::cout << vars[0].toInt() << '\n';
	}
	void EXPORT exit2(const std::vector<Variable>& vars)
	{
		exit(3);
	}
	void EXPORT cin(const std::vector<Variable>& vars)
	{
		std::string s;
		std::cin >> s;
		int i = std::stoi(s);
		*((int*)vars[0].data) = i;
	}
}
