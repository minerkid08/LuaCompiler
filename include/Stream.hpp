#pragma once

template <typename T> class Stream
{
  public:
	Stream()
	{
	}
	Stream(T* ptr2)
	{
		ptr = ptr2;
	}
	~Stream()
	{
		delete[] ptr;
	}
	void setPtr(T* ptr2)
	{
		ptr = ptr2;
	}
	const T& get(int offset = 0)
	{
		return ptr[i + offset];
	}
	const T& consume()
	{
		return ptr[++i];
	}
	void consume(int num)
	{
		i += num;
	}

  private:
	T* ptr;
	int i = 0;
};
