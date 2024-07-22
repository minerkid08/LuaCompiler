#include <FileUtils.hpp>

void writeInt(int number, FILE* file)
{
	fputc((number >> (8 * 0)) & 0xff, file);
	fputc((number >> (8 * 1)) & 0xff, file);
	fputc((number >> (8 * 2)) & 0xff, file);
	fputc((number >> (8 * 3)) & 0xff, file);
}

int readInt(Stream<char>& data)
{
	unsigned char c2 = data.get(4);
	int val = 0;
	val |= c2;
	c2 = data.get(3);
	val = val << 8;
	val |= c2;
	c2 = data.get(2);
	val = val << 8;
	val |= c2;
	c2 = data.get(1);
	val = val << 8;
	val |= c2;
  data.consume(4);
  return val;
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

std::string readString(Stream<char> &stream)
{
  char c = stream.consume();
  std::string str;
  while(c != 0)
  {
    str += c;
    c = stream.consume();
  }
  return str;
}

void writeString(const std::string &str, FILE *file)
{
  fputs(str.c_str(), file);
  fputc(0, file);
}
