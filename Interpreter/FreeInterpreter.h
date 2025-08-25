#pragma once
#include <stdbool.h>
#include "Interpreter.h"

void FreeValue(Value* Val);

void FreeReturnType(FunctionReturnType* RetType);

void FreeVariable(Variable* Var);

void FreeFunction(Function* Func);

void FreeEnvironment(VariableEnvironment* Env, bool AlsoParents);

void FreeAll(VariableEnvironment* Env);