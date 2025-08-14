#include "../Parser/Parser.h"

typedef enum E_VariableType {
	TYPE_INT,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_ARRAY,
	TYPE_STRUCT,
	TYPE_VOID,
} ValueType;

typedef struct S_Value {
	ValueType Type;
	union {
		int IntValue;
		double DoubleValue;
		char* StringValue;
		bool BoolValue;
		struct Value* ArrayValues;
		//Struct to be implemented
	};
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
	struct VariableEnvironment* ParentEnvironment;
} VariableEnvironment;

Value ExecuteExpression(Expression* Expr, VariableEnvironment* Env);

void Execute();