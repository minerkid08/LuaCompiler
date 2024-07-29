#include <iostream>
#include <vector>

#define EXPORT __declspec(dllexport)

enum class VarType
{
	Number,
	NumberRef
};

struct Variable
{
	VarType type;
	void* data;
};

int getInt(const Variable& var)
{
	long long l = (long long)var.data;
	return l;
}

extern "C"
{
	void EXPORT print(const std::vector<Variable>& vars)
	{
		std::cout << getInt(vars[0]);
	}
	void EXPORT println(const std::vector<Variable>& vars)
	{
		std::cout << getInt(vars[0]) << '\n';
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
