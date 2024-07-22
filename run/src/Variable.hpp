#pragma once

enum class VarType
{
	Number
};

struct Variable
{
	VarType type;
	int data;
	Variable()
	{
	}
	Variable(int n)
	{
		data = n;
	}
	Variable(VarType t, int n)
	{
		type = t;
		data = n;
	}
};
