#include "InterpreterPrintHelper.h"

//========PRINTING METHODS========

//Print methodS from chatgpt. Later moved into PrintHelpers
void PrintValue(Value v) {
	switch (v.Type) {
	case TYPE_INT:
		printf("%d", v.IntValue);
		break;

	case TYPE_DOUBLE:
		printf("%f", v.DoubleValue);
		break;

	case TYPE_STRING:
	case TYPE_IDENTIFIER:
		printf("\"%s\"", v.StringValue ? v.StringValue : "(null)");
		break;

	case TYPE_BOOL:
		printf(v.BoolValue ? "true" : "false");
		break;

	case TYPE_ARRAY:
	case TYPE_RETURN:
		printf("[");
		if (v.ArrayValues) {

			for (int i = 0; i < v.ArrayValuesLastIndex; i++)
			{
				PrintValue(v.ArrayValues[i]);
				if (i < v.ArrayValuesLastIndex - 1) printf(",");
			}
		}
		printf("]");
		break;

	case TYPE_STRUCT:
		printf("{ /* struct values here */ }");
		break;

	case TYPE_VOID:
		printf("void");
		break;

	default:
		printf("<unknown type>");
		break;
	}
}

const char* ValueTypeToString(ValueType type) {
	switch (type) {
	case TYPE_INT:    return "int";
	case TYPE_DOUBLE: return "double";
	case TYPE_STRING: return "string";
	case TYPE_BOOL:   return "bool";
	case TYPE_ARRAY:  return "array";
	case TYPE_RETURN: return "return";
	case TYPE_STRUCT: return "struct";
	case TYPE_VOID:   return "void";
	default:          return "<unknown>";
	}
}

void PrintVariable(const Variable* var) {
	if (!var) return;

	printf("Variable Name: %s\n", var->VariableName ? var->VariableName : "(null)");
	printf("  Forced Type: %s\n", ValueTypeToString(var->ForcedValueType));
	printf("  Value Type:  %s\n", ValueTypeToString(var->VariableValue.Type));
	printf("  Value: ");
	PrintValue(var->VariableValue);
	printf("\n");
}

void PrintFunction(const Function* func) {
	if (!func) return;

	printf("\033[1;32m=== Function ===\033[0m\n");
	printf("Function Name: %s\n", func->FunctionName ? func->FunctionName : "(null)");

	// Return types
	printf("Return Types (%d): ", func->ReturnTypesN);
	if (func->ReturnTypes && func->ReturnTypesN > 0) {
		for (int i = 0; i < func->ReturnTypesN; i++) {
			printf("%s", ValueTypeToString(func->ReturnTypes[i].Type));
			if (i < func->ReturnTypesN - 1) printf(", ");
		}
	}
	else {
		printf("(none)");
	}
	printf("\n");

	// Arguments
	printf("Arguments (%d):\n", func->ArgumentsN);
	for (int i = 0; i < func->ArgumentsN; i++) {
		printf("  Arg[%d] ", i);
		PrintVariable(&func->FuncEnvironment.Variables[i]);
	}

	// Environment
	printf("\n\033[1;34m--- Function Environment ---\033[0m\n");
	PrintVariableEnvironment(&func->FuncEnvironment);

	// Last output value
	printf("\nLastOutValue: ");
	PrintValue(func->LastOutValue);
	printf("\n");

	printf("\033[1;32m=== End of Function ===\033[0m\n");
}

void PrintVariableEnvironment(const VariableEnvironment* env) {
	if (!env) return;

	printf("\n\033[1;34m=== Variable Environment ===\033[0m\n");
	printf("Variables count: %d\n", env->LastVarIndex);

	for (int i = 0; i < env->LastVarIndex; i++) {
		printf("[%d] ", i);
		PrintVariable(&env->Variables[i]);
	}

	printf("Functions count: %d\n", env->LastFuncIndex);

	for (int i = 0; i < env->LastFuncIndex; i++)
	{
		printf("[%d] ", i);
		PrintFunction(&env->Functions[i]);
	}
	/*
	if (env->ParentEnvironment) {
		printf("--- Parent Environment ---\n");
		PrintVariableEnvironment(env->ParentEnvironment);
	}*/
	printf("\033[1;34m=== End of Variable Environment ===\033[0m\n");
}
