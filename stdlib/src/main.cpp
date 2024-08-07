#include <iostream>
#include <vector>

#include "Utils.hpp"
#include "Variable.hpp"

#define EXPORT __declspec(dllexport)

extern "C"
{
	void EXPORT print(const std::vector<Variable>& vars)
	{
		std::cout << vars[0].toString();
	}
	void EXPORT println(const std::vector<Variable>& vars)
	{
		std::cout << vars[0].toString() << '\n';
	}
	void EXPORT exit2(const std::vector<Variable>& vars)
	{
		exit(3);
	}
	void EXPORT cin(const std::vector<Variable>& vars)
	{
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
	void EXPORT type(const std::vector<Variable>& vars)
	{
		if (vars.size() != 2)
		{
			err("type requires 2 params, got " + std::to_string(vars.size()));
		}
		const Variable& v = vars[0];
		Variable* v2 = vars[1].deref();
		switch (v.type)
		{
		case VarType::Number:
			v2->resign("number");
			break;
		case VarType::String:
			v2->resign("string");
			break;
		case VarType::Ptr:
			v2->resign("ptr");
			break;
		case VarType::Opration:
			break;
		}
	}
}
