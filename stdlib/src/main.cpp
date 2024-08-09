#include <iostream>
#include <vector>

#include "Utils.hpp"
#include "Variable.hpp"

#define EXPORT extern "C" void __declspec(dllexport)

const char* strType(VarType type)
{
	switch (type)
	{
	case VarType::Number:
		return "number";
	case VarType::String:
		return "string";
	case VarType::Ptr:
		return "ptr";
	case VarType::Opration:
		return "error-type";
	}
	return "error-type";
}

void typeCheck(const Variable& var, VarType type, const char* msg)
{
	if (var.type != type)
		err(("var " + std::string(msg) + " expected: " + strType(type) + ", got: " + strType(var.type)));
}

EXPORT print(const std::vector<Variable>& vars)
{
	std::cout << vars[0].toString();
}

EXPORT println(const std::vector<Variable>& vars)
{
	std::cout << vars[0].toString() << '\n';
}

EXPORT exit2(const std::vector<Variable>& vars)
{
	exit(3);
}

EXPORT cin(const std::vector<Variable>& vars)
{
	typeCheck(vars[0], VarType::Ptr, "arg 1");
	std::string s;
	std::cin >> s;
	bool num = true;
	for (char c : s)
	{
		if ((c < '0' || c > '9') && c != '.')
		{
			num = false;
			break;
		}
	}
	Variable* v = vars[0].deref();
	if (num)
	{
		v->resign(std::stoi(s));
	}
	else
	{
		v->resign(s);
	}
}

EXPORT type(const std::vector<Variable>& vars)
{
	if (vars.size() != 2)
	{
		err("type requires 2 params, got " + std::to_string(vars.size()));
	}
	typeCheck(vars[1], VarType::Ptr, "arg 2");
	const Variable& v = vars[0];
	Variable* v2 = vars[1].deref();
	v2->resign(strType(v.type));
}
