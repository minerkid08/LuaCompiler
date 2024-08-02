#pragma once

enum class VarType
{
	Number,
	Ptr,
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
	inline Variable(Variable* n)
	{
		type = VarType::Ptr;
		data = (void*)n;
	}
	inline int geti() const
	{
		long long d = (long long)data;
		return (int)d;
	}
	inline Variable deref() const
	{
		if (type == VarType::Ptr)
			return *(Variable*)data;
		return geti();
	}
	inline int toInt() const
	{
		if (type == VarType::Ptr)
			return deref().geti();
		return geti();
	}
};
