#pragma once
#include "Interpreter.h"

void PrintValue(Value v);

void PrintOutValue(Value v);

const char* ValueTypeToString(ValueType type);

void PrintVariable(const Variable* var);

void PrintFunction(const Function* func);

void PrintVariableEnvironment(const VariableEnvironment* env);