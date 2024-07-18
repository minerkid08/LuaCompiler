#pragma once

#include <vector>

template <typename T> class StreamVec
{
  public:
	void push(const T& item)
	{
		vec.push_back(item);
	}
	const T* get(int offset = 0)
	{
		return &(vec[i + offset]);
	}
	void consume(int num)
	{
		i += num;
	}
	const T* consume()
	{
		return &(vec[++i]);
	}

  private:
	int i = 0;
	std::vector<T> vec;
};
