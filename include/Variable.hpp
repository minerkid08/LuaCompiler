#pragma once
#include <iostream>
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
	VarType type = VarType::Number;
	void* data = nullptr;

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
		std::cout << str << '\n';
		std::cout << str.size() << '\n';
		std::string* s = new std::string();
		for (char c : str)
			s->push_back(c);
		std::cout << s->size() << '\n';
		data = (void*)s;
	}

	inline Variable(const Variable& v)
	{
		type = v.type;
		if (v.type == VarType::String)
		{
			std::cout << "copying string\n";
			if (v.data == nullptr)
				std::cout << "nullptr:(\n";
			std::cout << v.data << '\n';
			const std::string& s2 = v.gets();
			std::cout << s2.size();
			std::cout << s2 << '\n';
			std::string* s = new std::string();
			for (char c : s2)
			{
				// std::cout << c << '\n';
				s->push_back(c);
			}
			data = (void*)s;
			std::cout << "copied string\n";
		}
		else
		{
			data = v.data;
		}
	}

	~Variable()
	{
		if (type == VarType::String)
		{
			std::string* str = (std::string*)data;
			delete str;
		}
	}

	inline int geti() const
	{
		return (int)(long long)data;
	}

	inline std::string& gets() const
	{
		return *(std::string*)data;
	}

	inline Variable* deref() const
	{
		if (type == VarType::Ptr)
			return (Variable*)data;
		return nullptr;
	}

	inline int toInt() const
	{
		if (type == VarType::Ptr)
			return deref()->geti();
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
