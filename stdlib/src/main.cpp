#include <iostream>
#include <vector>

#define EXPORT __declspec(dllexport)

enum class VarType
{
	Number
};

struct Variable
{
	VarType type;
	int data;
};

extern "C"
{
	void EXPORT print(const std::vector<Variable>& vars)
	{
		std::cout << vars[0].data;
	}
	void EXPORT println(const std::vector<Variable>& vars)
	{
		std::cout << vars[0].data << '\n';
	}
}
