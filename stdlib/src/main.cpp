#include <iostream>

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
	void EXPORT print(const Variable& var)
	{
		std::cout << var.data;
	}
	void EXPORT println(const Variable& var)
	{
		std::cout << var.data << '\n';
	}
}
