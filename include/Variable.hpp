#pragma once
#include <string>

enum class VarType
{
	Number,
	String,
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
	inline Variable(const std::string& str)
	{
		type = VarType::String;
		data = (void*)new std::string(str);
	}
	inline Variable(const Variable& v)
	{
		type = v.type;
		if (v.type == VarType::String)
		{
			data = (void*)new std::string(v.gets());
		}
		else
		{
			data = v.data;
		}
	}
	~Variable()
	{
		if (type == VarType::String)
			delete (std::string*)data;
	}
	inline int geti() const
	{
		return (int)(long long)data;
	}
	inline std::string& gets() const
	{
		return *(std::string*)data;
	}
	inline Variable deref() const
	{
		if (type == VarType::Ptr)
			return *(Variable*)data;
		return nullptr;
	}
	inline int toInt() const
	{
		if (type == VarType::Ptr)
			return deref().geti();
		return geti();
	}
	inline std::string toString() const
	{
		if (type == VarType::String)
			return *((std::string*)data);
		else
			return std::to_string((long long)data);
	}
};
