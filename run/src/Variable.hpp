#pragma once

enum class VarType
{
	Number,
	NumberPtr
};

struct Variable
{
	VarType type;
	void* data;
	Variable()
	{
	}
	Variable(int n)
	{
    type = VarType::Number;
    long long n2 = n;
		data = (void*)n2;
	}
	Variable(int* n)
	{
		type = VarType::NumberPtr;
		data = (void*)n;
	}
	int geti()
	{
    long long d = (long long)data;
		return (int)d;
	}
};
