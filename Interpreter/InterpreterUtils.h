#pragma once
#include "Interpreter.h"

void AdvanceExpression();

bool OperateStringValues(char* LValue, char* RValue, BinaryExpressionType Operator);

int OperateIntValues(int LValue, int RValue, BinaryExpressionType Operator, bool* IsBool);

double OperateDoubleValues(double LValue, double RValue, BinaryExpressionType Operator, bool* IsBool);

VariableEnvironment CreateEmptyEnvironment(VariableEnvironment* Parent);

Variable* VarSearchEnvironment(char* VarName, VariableEnvironment* Env);

Function* FunctionSearchEnvironment(char* FuncName, VariableEnvironment* Env);

void AddVariableToEnvironment(Variable* Var, VariableEnvironment* Env);

void AddFunctionToEnvironment(Function* Func, VariableEnvironment* Env);

ValueType GetForcedVariableType(DeclarationVariableType Type);

void AssignStringHelper(Variable* Var, Value Val);

void AssignIntHelper(Variable* Var, Value Val);

void AssignDoubleHelper(Variable* Var, Value Val);

void AssignArrayHelper(Variable* Var, Value Val);

void AssignVariableValue(Variable* Var, Value Val);

ValueType GetFuncReturnType(FunctionReturnInfo* Info, Value* StructCaseName, VariableEnvironment* Env);

bool CheckForIdentifierVariable(Value* Val, VariableEnvironment* Env, Value* OutVal);