#define _CRT_SECURE_NO_WARNINGS
#include "InterpreterUtils.h"
#include "Interpreter.h"

//Goes to the next expression in the global linked list.
// If it's the last expression, sets EndOfExpressions to true.
void AdvanceExpression() {
	if (ExprFirst == NULL) {
		EndOfExpressions = true;
		return; //Add Errors
	}
	CurrExpression = ExprFirst->Expr;
	ExprFirst = ExprFirst->Next;
}

//Handles binary operations between two strings.
bool OperateStringValues(char* LValue, char* RValue, BinaryExpressionType Operator) {
	if (LValue == NULL || RValue == NULL) return false;

	int Compared = strcmp(LValue, RValue);

	switch (Operator) {
	case BINARY_LESS:     return Compared < 0;
	case BINARY_GREATER:  return Compared > 0;
	case BINARY_LOE:      return Compared <= 0;
	case BINARY_GOE:      return Compared >= 0;
	case BINARY_EQUAL:    return Compared == 0;
	case BINARY_NOE:      return Compared != 0;
	case BINARY_AND:      return *LValue && *RValue;
	case BINARY_OR:       return *LValue || *RValue;
	default:
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteNode: Not allowed string operation." });
		FreeAll(&GlobalEnvironment);
		return false;
	}
}

//Handles binary operations between two integers.
int OperateIntValues(int LValue, int RValue, BinaryExpressionType Operator, bool* IsBool) {
	if (Operator != BINARY_ADD && Operator != BINARY_SUB) *IsBool = true;
	switch (Operator)
	{
	case BINARY_ADD: return LValue + RValue;
	case BINARY_SUB: return LValue - RValue;
	case BINARY_LESS: return LValue < RValue;
	case BINARY_GREATER: return LValue > RValue;
	case BINARY_LOE: return LValue <= RValue;
	case BINARY_GOE: return LValue >= RValue;
	case BINARY_EQUAL: return LValue == RValue;
	case BINARY_NOE: return LValue != RValue;
	case BINARY_AND: return LValue && RValue;
	case BINARY_OR: return LValue || RValue;
	default:
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteNdoe: Unknown node." });
		FreeAll(&GlobalEnvironment);
		return 0;
	}
}

//Handles binary operations between two doubles.
double OperateDoubleValues(double LValue, double RValue, BinaryExpressionType Operator, bool* IsBool) {
	if (Operator != BINARY_ADD && Operator != BINARY_SUB) *IsBool = true;
	switch (Operator)
	{
	case BINARY_ADD: return LValue + RValue;
	case BINARY_SUB: return LValue - RValue;
	case BINARY_LESS: return LValue < RValue;
	case BINARY_GREATER: return LValue > RValue;
	case BINARY_LOE: return LValue <= RValue;
	case BINARY_GOE: return LValue >= RValue;
	case BINARY_EQUAL: return LValue == RValue;
	case BINARY_NOE: return LValue != RValue;
	case BINARY_AND: return LValue && RValue;
	case BINARY_OR: return LValue || RValue;
	default:
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteNdoe: Unknown node." });
		FreeAll(&GlobalEnvironment);
		return 0.0f;
	}
}

//Creates an empty variable environment, mallocs variables and functions arrays.
// Parent can be NULL.
VariableEnvironment CreateEmptyEnvironment(VariableEnvironment* Parent) {
	Variable* Vars = malloc(sizeof(Variable));
	if (Vars == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in CreateEmptyEnv: Vars malloc failed." });
		FreeAll(&GlobalEnvironment);
	}


	Function* Functions = malloc(sizeof(Function));
	if (Functions == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in CreateEmptyEnv: Functions malloc failed." });
		free(Vars);
		Vars = NULL;
		FreeAll(&GlobalEnvironment);
	}

	return (VariableEnvironment) { Vars, .VariablesSize = 1, .LastVarIndex = 0, Functions, .FunctionsSize = 1, .LastFuncIndex = 0, Parent };
}

//Searches for a variable (by varname) in the environment and its parents. Returns NULL if not found.
Variable* VarSearchEnvironment(char* VarName, VariableEnvironment* Env) {
	VariableEnvironment* CurrEnv = Env;

	while (CurrEnv != NULL)
	{
		for (int i = 0; i < CurrEnv->LastVarIndex; i++)
		{
			if (strcmp(VarName, CurrEnv->Variables[i].VariableName) == 0) {
				return &(CurrEnv->Variables[i]);
			}
		}
		CurrEnv = CurrEnv->ParentEnvironment;
	}

	return NULL;
}

//Same as VarSearchEnvironment but for functions.
Function* FunctionSearchEnvironment(char* FuncName, VariableEnvironment* Env) {
	VariableEnvironment* CurrEnv = Env;

	while (CurrEnv != NULL)
	{
		for (int i = 0; i < CurrEnv->LastFuncIndex; i++)
		{
			if (strcmp(FuncName, CurrEnv->Functions[i].FunctionName) == 0) {
				return &(CurrEnv->Functions[i]);
			}
		}
		CurrEnv = CurrEnv->ParentEnvironment;
	}

	return NULL;
}

//Adds a variable to the environment, reallocs if needed. (remember to increase space by (ex) 10, not just 1.
//Still, for clarity, do it by 1 for now.
void AddVariableToEnvironment(Variable* Var, VariableEnvironment* Env) {
	if (VarSearchEnvironment(Var->VariableName, Env) != NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddVariableToEnvironment: Variable name already exists in this scope." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	Env->Variables[Env->LastVarIndex] = *Var;
	Env->LastVarIndex++;

	if (Env->LastVarIndex >= Env->VariablesSize) {
		Env->VariablesSize += 10;

		Variable* Temp = realloc(Env->Variables, sizeof(Variable) * Env->VariablesSize);

		if (Temp == NULL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddVariableToEnvironment: Vars realloc failed." });
			FreeAll(&GlobalEnvironment);
			return;
		}
		Env->Variables = Temp;
		if (Env->Variables == NULL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddVariableToEnvironment: Vars malloc failed." });
			FreeAll(&GlobalEnvironment);
			return;
		}
	}
}

//Same as AddVariableToEnvironment but for functions.
void AddFunctionToEnvironment(Function* Func, VariableEnvironment* Env) {
	if (FunctionSearchEnvironment(Func->FunctionName, Env) != NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddFunctionToEnvironment: Function name already exists in this scope." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	Env->Functions[Env->LastFuncIndex] = *Func;
	Env->LastFuncIndex++;

	if (Env->LastFuncIndex >= Env->FunctionsSize) {
		Env->FunctionsSize += 10;

		Function* Temp = realloc(Env->Functions, sizeof(Function) * Env->FunctionsSize);
		if (Temp == NULL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddFunctionToEnvironment: Funcs realloc failed." });
			FreeAll(&GlobalEnvironment);
			return;
		}

		Env->Functions = Temp;
		if (Env->Functions == NULL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddFunctionToEnvironment: Functions malloc failed." });
			FreeAll(&GlobalEnvironment);
			return;
		}
	}
}

//For immediate type declaration (not auto), returns correct forced type.
//Returns -1 if automatic type.
ValueType GetForcedVariableType(DeclarationVariableType Type) {

	switch (Type)
	{
	case VARIABLE_INT: return TYPE_INT;
	case VARIABLE_DOUBLE: return TYPE_DOUBLE;
	case VARIABLE_STRING: return TYPE_STRING;
	case VARIABLE_BOOL: return TYPE_BOOL;
	case VARIABLE_ARRAY: return TYPE_ARRAY;
	case VARIABLE_CUSTOM: return TYPE_STRUCT;
	case VARIABLE_AUTO: return -1;
	case VARIABLE_NONE:	return TYPE_VOID;
	default:
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in GetForcedVariableType: unknown variable type." });
		FreeAll(&GlobalEnvironment);
		return TYPE_VOID;
	}
}

//Helper for string assignment. Called in AssignVariableValue.
void AssignStringHelper(Variable* Var, Value Val) {
	Var->VariableValue.Type = TYPE_STRING;
	if (Val.Type != TYPE_STRING) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AssignVariableValue: Tried to assign non string to string." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	char* Temp = realloc(Var->VariableValue.StringValue, sizeof(char) * (strlen(Val.StringValue) + 1));
	if (Temp == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AssignVariableValue: Realloc failed." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	Var->VariableValue.StringValue = Temp;
	if (Var->VariableValue.StringValue == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AssignVariableValue: StringValue malloc failed." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	strcpy(Var->VariableValue.StringValue, Val.StringValue);
}

//Helper for int assignment. Called in AssignVariableValue.	
void AssignIntHelper(Variable* Var, Value Val) {
	if (Val.Type == TYPE_INT) {
		Var->VariableValue.Type = TYPE_INT;
		Var->VariableValue.IntValue = Val.IntValue;
	}
	else if (Val.Type == TYPE_DOUBLE) Var->VariableValue.IntValue = (int)Val.DoubleValue;
	else {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AssignVariableValue: Tried to assign non int/double to int." });
		FreeAll(&GlobalEnvironment);
		return;
	}
}

//Helper for double assignment. Called in AssignVariableValue.
void AssignDoubleHelper(Variable* Var, Value Val) {
	Var->VariableValue.Type = TYPE_DOUBLE;
	if (Val.Type == TYPE_INT) Var->VariableValue.DoubleValue = (double)Val.IntValue;
	else if (Val.Type == TYPE_DOUBLE) Var->VariableValue.DoubleValue = Val.DoubleValue;
	else {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AssignVariableValue: Tried to assign non int/double to double." });
		FreeAll(&GlobalEnvironment);
		return;
	}
}

//Helper for array assignment. Called in AssignVariableValue.
void AssignArrayHelper(Variable* Var, Value Val) {
	Var->VariableValue.Type = TYPE_ARRAY;
	if (Val.Type != TYPE_ARRAY) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AssignVariableValue: Tried to assign non non array to array." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	Var->VariableValue = Val;
}

//Assigns a value to a variable, checking for type coherence.
void AssignVariableValue(Variable* Var, Value Val) {
	switch (Var->ForcedValueType)
	{
	case TYPE_INT:
		AssignIntHelper(Var, Val);
		return;

	case TYPE_DOUBLE:
		AssignDoubleHelper(Var, Val);
		return;

	case TYPE_STRING:
		AssignStringHelper(Var, Val);
		return;

	case TYPE_ARRAY:
		AssignArrayHelper(Var, Val);
		return;

		//case TYPE_BOOL: 
		//case TYPE_STRUCT: 
		//case TYPE_VOID:
	case -1:
		Var->VariableValue = Val;
		return;
	}

}

//Returns the ValueType corresponding to the FunctionReturnInfo.
ValueType GetFuncReturnType(FunctionReturnInfo* Info, Value* StructCaseName, VariableEnvironment* Env) {
	switch (Info->Type) {
	case FUNCTION_INT:    return TYPE_INT;
	case FUNCTION_DOUBLE: return TYPE_DOUBLE;
	case FUNCTION_STRING: return TYPE_STRING;
	case FUNCTION_BOOL:   return TYPE_BOOL;
	case FUNCTION_ARRAY:  return TYPE_ARRAY;
	case FUNCTION_CUSTOM:
		if (Info->Value != NULL) {
			*StructCaseName = ExecuteExpression(Info->Value, Env);
			return TYPE_STRUCT;
		}
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in GetFuncReturnType: Struct not recognized." });
		FreeAll(&GlobalEnvironment);
		return TYPE_VOID;

	case FUNCTION_VOID:   return TYPE_VOID;
	default:
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in GetFuncReturnType: Type not recognized." });
		FreeAll(&GlobalEnvironment);
		return TYPE_VOID;
	}
}

//If Val is an identifier, searches for it in the environment (and parents) and if found, sets OutVal to its value and returns true.
//Used to dereference identifiers before operations.
bool CheckForIdentifierVariable(Value* Val, VariableEnvironment* Env, Value* OutVal) {
	if (Val->Type == TYPE_IDENTIFIER) {
		Variable* Check = VarSearchEnvironment(Val->StringValue, Env);
		if (Check == NULL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteBinary: Identifier not recognized." });
			FreeAll(&GlobalEnvironment);
			return false;
		}
		*OutVal = Check->VariableValue;
		return true;
	}
	return false;
}