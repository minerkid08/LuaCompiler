#pragma once

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
