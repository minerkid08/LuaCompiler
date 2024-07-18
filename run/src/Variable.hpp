#pragma once

enum class VarType
{
  Number
};

struct Variable
{
  VarType type;
  int data;
};
