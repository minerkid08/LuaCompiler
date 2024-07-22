#pragma once

#include <cstdio>
#include <string>

#include "Stream.hpp"

int readInt(Stream<char>& stream);
int readInt(int &i, const char* data);
void writeInt(int num, FILE* file);

std::string readString(Stream<char>& stream);
void writeString(const std::string& str, FILE* file);
