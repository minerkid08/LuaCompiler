#include <iostream>
#include <vector>

#define EXPORT __declspec(dllexport)

enum class VarType
{
	Number,
	NumberPtr,
	Opration
};

struct Variable
{
	VarType type;
	void* data;
	inline Variable()
	{
	}
	inline Variable(int n, VarType t = VarType::Number)
	{
		type = t;
		long long n2 = n;
		data = (void*)n2;
	}
	inline Variable(int* n)
	{
		type = VarType::NumberPtr;
		data = (void*)n;
	}
	inline int geti() const
	{
		long long d = (long long)data;
		return (int)d;
	}
	inline int toInt() const
	{
		if (type == VarType::NumberPtr)
			return *(int*)data;
		return geti();
	}
};

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
