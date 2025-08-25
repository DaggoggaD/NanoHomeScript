#pragma once
#include "../Parser/Parser.h"


//========ENUMS========
typedef enum E_VariableType {
	TYPE_INT,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_ARRAY,
	TYPE_RETURN,
	TYPE_STRUCT,
	TYPE_VOID,
	TYPE_IDENTIFIER
} ValueType;

//========STRUCTS========
typedef struct S_Value {
	ValueType Type;
	union {
		int IntValue;
		double DoubleValue;
		char* StringValue;
		bool BoolValue;
		struct S_Value* ArrayValues; //ALSO FOR RETURNS
		//Struct to be implemented
	};
	int ArrayValuesLastIndex;
	int ArrayValuesSize;
} Value;

typedef struct S_Variable {
	char* VariableName;
	ValueType ForcedValueType;
	Value VariableValue;
} Variable;

typedef struct S_VariableEnvironment {
	Variable* Variables;
	int VariablesSize;
	int LastVarIndex;

	struct S_Function* Functions;
	int FunctionsSize;
	int LastFuncIndex;

	struct VariableEnvironment* ParentEnvironment;
} VariableEnvironment;

typedef struct S_FunctionReturnType {
	ValueType Type;
	Value StructCaseName;
} FunctionReturnType;

typedef struct S_Function {
	char* FunctionName;

	FunctionReturnType* ReturnTypes;
	int ReturnTypesN;

	VariableEnvironment FuncEnvironment; //The first ArgumentsN elements are the arguments
	int ArgumentsN;

	Expression* ExpressionsBlock;
	Value LastOutValue;

} Function;

//========GLOBAL VARIABLES========

extern Expression* CurrExpression;
extern bool EndOfExpressions;
extern VariableEnvironment GlobalEnvironment;

//========FUNCTIONS========

Value ExecuteExpression(Expression* Expr, VariableEnvironment* Env);

void Execute();