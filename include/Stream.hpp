#pragma once

template <typename T> class Stream
{
  public:
	Stream(T* ptr2)
	{
		ptr = ptr2;
	}
	~Stream()
	{
		return ptr;
	}
	const T& get(int offset)
	{
		return ptr[i + offset];
	}
	void consume(int num)
	{
		i += num;
	}

  private:
	T* ptr;
	int i = 0;
};
