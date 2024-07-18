#pragma once

#include <vector>

template <typename T> class Stack
{
  public:
	void push(const T& t)
	{
		vec.push_back(t);
	}
	void pop(int num = 1)
	{
		vec.resize(vec.size() - num);
	}
	void popTo(int count)
	{
		vec.resize(count);
	}
	T& operator[](int index)
	{
		return vec[index];
	}
	std::vector<T>& getVec()
	{
		return vec;
	}
	bool contains(const T& value)
	{
		for (const T& item : vec)
		{
			if (item == value)
			{
				return true;
			}
		}
		return false;
	}
	int size()
	{
		return vec.size();
	}
	T& top()
	{
		return vec[vec.size() - 1];
	}
	auto begin()
	{
		return vec.begin();
	}
	auto begin() const
	{
		return vec.begin();
	}
	auto end()
	{
		return vec.begin();
	}
	auto end() const
	{
		return vec.begin();
	}

  private:
	std::vector<T> vec;
};
