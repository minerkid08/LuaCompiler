#include <FileUtils.hpp>

void writeInt(int number, FILE* file)
{
	fputc((number >> (8 * 0)) & 0xff, file);
	fputc((number >> (8 * 1)) & 0xff, file);
	fputc((number >> (8 * 2)) & 0xff, file);
	fputc((number >> (8 * 3)) & 0xff, file);
}

int readInt(int& i, const char* data)
{
	unsigned char c2 = data[i + 4];
	int val = 0;
	val |= c2;
	c2 = data[i + 3];
	val = val << 8;
	val |= c2;
	c2 = data[i + 2];
	val = val << 8;
	val |= c2;
	c2 = data[i + 1];
	val = val << 8;
	val |= c2;
	i += 4;
  return val;
}
