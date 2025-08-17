#define _CRT_SECURE_NO_WARNINGS
#include "Interpreter.h"
#include "../Utils/PrintHelpers.h"

Expression* CurrExpression;
bool EndOfExpressions = false;
VariableEnvironment GlobalEnvironment;

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
		printf("[");
		if (v.ArrayValues) {
			// Qui servirebbe sapere la lunghezza dell'array
			// per ora facciamo un placeholder
			printf("/* array values here */");
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

///END OF PRINTING METHODS

//========INTERPRETER UTILITIES========

void AdvanceExpression() {
	if (ExprFirst == NULL) {
		EndOfExpressions = true;
		return; //Add Errors
	}
	CurrExpression = ExprFirst->Expr;
	ExprFirst = ExprFirst->Next;
}

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
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteNode: Not allowed string operation." });
		return false;
	}
}

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
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteNdoe: Unknown node." });
		return 0;
	}
}

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
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteNdoe: Unknown node." });
		return 0.0f;
	}
}

VariableEnvironment CreateEmptyEnvironment(VariableEnvironment *Parent) {
	Variable* Vars = malloc(sizeof(Variable));
	if(Vars == NULL) PrintGrammarError((GrammarError) { 0, 0, "Error in CreateEmptyEnv: Vars malloc failed." });

	Function* Functions = malloc(sizeof(Function));
	if(Functions == NULL)PrintGrammarError((GrammarError) { 0, 0, "Error in CreateEmptyEnv: Functions malloc failed." });

	return (VariableEnvironment) {Vars, .VariablesSize = 1, .LastVarIndex = 0, Functions, .FunctionsSize = 1, .LastFuncIndex=0,Parent };
}

void AddVariableToEnvironment(Variable *Var, VariableEnvironment *Env) {
	Env->Variables[Env->LastVarIndex] = *Var;
	Env->LastVarIndex++;

	//Add search parent to check if name already exists

	if (Env->LastVarIndex >= Env->VariablesSize) {
		Env->VariablesSize++;

		Variable* Temp = realloc(Env->Variables, sizeof(Variable) * Env->VariablesSize);

		if (Temp == NULL) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AddVariableToEnvironment: Vars realloc failed." });
			return;
		}
		Env->Variables = Temp;
		if(Env->Variables == NULL) PrintGrammarError((GrammarError) { 0, 0, "Error in AddVariableToEnvironment: Vars malloc failed." });
	}
}

void AddFunctionToEnvironment(Function *Func, VariableEnvironment *Env) {
	Env->Functions[Env->LastFuncIndex] = *Func;
	Env->LastFuncIndex++;

	//Add search parent to check if name already exists

	if (Env->LastFuncIndex >= Env->FunctionsSize) {
		Env->FunctionsSize++;

		Function* Temp = realloc(Env->Functions, sizeof(Function) * Env->FunctionsSize);
		if (Temp == NULL) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AddFunctionToEnvironment: Funcs realloc failed." });
			return;
		}

		Env->Functions = Temp;
		if (Env->Functions == NULL) PrintGrammarError((GrammarError) { 0, 0, "Error in AddFunctionToEnvironment: Functions malloc failed." });
	}
}

Variable* VarSearchEnvironment(char* VarName, VariableEnvironment *Env){
	VariableEnvironment* CurrEnv = Env;

	while (CurrEnv!=NULL)
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
		PrintGrammarError((GrammarError) { 0, 0, "Error in GetForcedVariableType: unknown variable type." });
		return -1;
		break;
	}
}

void AssignVariableValue(Variable* Var, Value Val) {

	switch (Var->ForcedValueType)
	{
	case TYPE_INT: 
		if (Val.Type == TYPE_INT) Var->VariableValue.IntValue = Val.IntValue;
		else if (Val.Type == TYPE_DOUBLE) Var->VariableValue.IntValue = (int)Val.DoubleValue;
		else {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AssignVariableValue: Tried to assign non int/double to int." });
			return;
		}
		break;

	case TYPE_DOUBLE: 
		if (Val.Type == TYPE_INT) Var->VariableValue.DoubleValue = (double)Val.IntValue;
		else if (Val.Type == TYPE_DOUBLE) Var->VariableValue.DoubleValue = Val.DoubleValue;
		else {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AssignVariableValue: Tried to assign non int/double to double." });
			return;
		}
		break;

	case TYPE_STRING: 
		if(Val.Type!=TYPE_STRING) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AssignVariableValue: Tried to assign non string to string." });
			return;
		}

		char* Temp = realloc(Var->VariableValue.StringValue, sizeof(char) * (strlen(Val.StringValue) + 1));
		if (Temp == NULL) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AssignVariableValue: Realloc failed." });
			return;
		}

		Var->VariableValue.StringValue = Temp;
		if (Var->VariableValue.StringValue == NULL) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in AssignVariableValue: StringValue malloc failed." });
			return;
		}

		strcpy(Var->VariableValue.StringValue, Val.StringValue);

	//case TYPE_BOOL: 

	//case TYPE_ARRAY:

	//case TYPE_STRUCT: 

	//case TYPE_VOID:
	case -1:
		Var->VariableValue = Val;
		break;
	}

}

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
		PrintGrammarError((GrammarError) { 0, 0, "Error in GetFuncReturnType: Struct not recognized." });
		return;
	case FUNCTION_VOID:   return TYPE_VOID;
	default:
		PrintGrammarError((GrammarError) { 0, 0, "Error in GetFuncReturnType: Type not recognized." });
		return;

	}
}

bool CheckForIdentifierVariable(Value* Val, VariableEnvironment* Env, Value* OutVal) {
	if (Val->Type == TYPE_IDENTIFIER) {
		Variable* Check = VarSearchEnvironment(Val->StringValue, Env);
		if (Check == NULL) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteBinary: Identifier not recognized." });
			return false;
		}

		*OutVal = Check->VariableValue;
		return true;
	}
	return false;
}

//========EXECUTION METHODS========

void ExecuteFunctionCall(char* FuncName, FunctionReturnInfo** Args, int ArgsN, VariableEnvironment* Env) {
	Function* Func = FunctionSearchEnvironment(FuncName, Env);

	if (Func == NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteFunctionCall: No such named function." });
		return;
	}

	if(ArgsN!=Func->ArgumentsN) PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteFunctionCall: Uncoherent number of argoments passed." });
	
	for (int i = 0; i < ArgsN; i++)
	{
		Value CurrArgValue = ExecuteExpression(Args[i]->Value, Env);
		Func->FuncEnvironment.Variables[i].VariableValue = CurrArgValue;
	}

	ExecuteExpression(Func->ExpressionsBlock, &Func->FuncEnvironment);
	PrintVariableEnvironment(&Func->FuncEnvironment);
}

Value ExecuteNode(Expression* Expr, VariableEnvironment *Env){
	Node* CurrNode = Expr->Value.NodeExpr;
	Value CurrVal;
	switch (CurrNode->Type)
	{
	case NODE_NUMBER:
		if (CurrNode->Value.Tok.Type == INT) {
			CurrVal.Type = TYPE_INT;
			CurrVal.IntValue = CurrNode->Value.Tok.Value.intVal;
			return CurrVal;
		}
		CurrVal.Type = TYPE_DOUBLE;
		CurrVal.DoubleValue = CurrNode->Value.Tok.Value.doubleVal;
		
		return CurrVal;
	case NODE_STRING:
		CurrVal.Type = TYPE_STRING;
		CurrVal.StringValue = malloc(sizeof(char) * (strlen(CurrNode->Value.Tok.Value.stringVal)+1));
		if (CurrVal.StringValue == NULL) {
			PrintGrammarError((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: CurrVal malloc failed." });
			return;
		}

		strcpy(CurrVal.StringValue, CurrNode->Value.Tok.Value.stringVal);
		return CurrVal;

	case NODE_GROUPING:
		return ExecuteExpression(CurrNode->Value.NodeGrouping, Env);

	//case NODE_BOOL:
	
	case NODE_IDENTIFIER:
		CurrVal.Type = TYPE_IDENTIFIER;
		CurrVal.StringValue = malloc(sizeof(char) * (strlen(CurrNode->Value.Tok.Value.stringVal)+1));
		if (CurrVal.StringValue == NULL) {
			PrintGrammarError((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: CurrVal malloc failed." });
			return (Value) { .Type = TYPE_VOID };;
		}

		strcpy(CurrVal.StringValue, CurrNode->Value.Tok.Value.stringVal);
		return CurrVal;

	case NODE_BLOCK:
		for (int i = 0; i < CurrNode->Value.Block.Index; i++)
		{
			ExecuteExpression(CurrNode->Value.Block.Expressions[i], Env);
		}
		return (Value) { .Type = TYPE_VOID }; //Maybe add a return for node block
	
	case NODE_CALL:
		ExecuteFunctionCall(CurrNode->Value.FuncCall.CallNameTok.Value.stringVal, CurrNode->Value.FuncCall.Arguments, CurrNode->Value.FuncCall.ArgumentsNamesCount, Env);
		return (Value) { .Type = TYPE_VOID };
	default:
		PrintGrammarError((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: Unknown node." });
		return (Value) { .Type = TYPE_VOID };
	}

}

Value ExecuteFactor(Expression* Expr, VariableEnvironment *Env) {
	Factor* CurrFactor = Expr->Value.FactorExpr;
	Value FactValue = ExecuteExpression(CurrFactor->Value, Env);
	
	if (CurrFactor->Type == FACTOR_NEGATIVE)
	{
		if (FactValue.Type == TYPE_INT) {
			FactValue.IntValue = -FactValue.IntValue;
		}
		else {
			FactValue.DoubleValue = -FactValue.DoubleValue;
		}
	}
	else if (CurrFactor->Type == FACTOR_NOT ) {
		if (FactValue.Type != TYPE_BOOL) PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteFactor: ! can't be performed on a non boolean." });
		FactValue.BoolValue = !FactValue.BoolValue;
	}

	return FactValue;
}

Value ExecuteTerm(Expression* Expr, VariableEnvironment* Env) {
	Term* CurrTerm = Expr->Value.TermExpr;
	Value Left = ExecuteExpression(CurrTerm->Left, Env);
	Value Right = ExecuteExpression(CurrTerm->Right, Env);
	Value OutValue;

	if (CurrTerm->Type == TERM_NONE) return Left;

	if ((Left.Type != TYPE_INT && Left.Type != TYPE_DOUBLE) || (Right.Type != TYPE_INT && Right.Type != TYPE_DOUBLE)) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteBinary: Binary operation on non binary-accepted values." });
		return;
	}

	if (Left.Type == TYPE_INT && Right.Type == TYPE_INT) {
		OutValue.Type = TYPE_INT;

		if (CurrTerm->Type == TERM_MOLTIPLICATION) OutValue.IntValue = Left.IntValue * Right.IntValue;
		else OutValue.IntValue = Left.IntValue / Right.IntValue;

		
	}
	else if (Left.Type == TYPE_DOUBLE || Right.Type == TYPE_DOUBLE) {
		OutValue.Type = TYPE_DOUBLE;
		double LV = (Left.Type == TYPE_INT) ? (double)Left.IntValue : Left.DoubleValue;
		double RV = (Right.Type == TYPE_INT) ? (double)Right.IntValue : Right.DoubleValue;

		if (CurrTerm->Type == TERM_MOLTIPLICATION) OutValue.DoubleValue = LV*RV;
		else OutValue.DoubleValue = LV/RV;
	}

	return OutValue;
}

Value ExecuteBinary(Expression* Expr, VariableEnvironment* Env) {
	BinaryExpression* CurrBinary = Expr->Value.BinExpr;
	Value Left = ExecuteExpression(CurrBinary->Left, Env);
	Value Right = ExecuteExpression(CurrBinary->Right, Env);

	CheckForIdentifierVariable(&Left, Env, &Left);
	CheckForIdentifierVariable(&Right, Env, &Right);

	Value OutValue;

	if ((Left.Type != TYPE_INT && Left.Type != TYPE_DOUBLE) || (Right.Type != TYPE_INT && Right.Type != TYPE_DOUBLE)) {
		if (Left.Type == TYPE_STRING && Right.Type == TYPE_STRING) {
			OutValue.Type = TYPE_BOOL;
			OutValue.BoolValue = OperateStringValues(Left.StringValue, Right.StringValue, CurrBinary->Type);
			return OutValue;
		}

		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteBinary: Binary operation on non binary-accepted values." });
		return;
	}
	
	bool IsBool = false;

	if (Left.Type == TYPE_INT && Right.Type == TYPE_INT) {
		OutValue.IntValue = OperateIntValues(Left.IntValue, Right.IntValue, CurrBinary->Type, &IsBool);
		if (IsBool) OutValue.Type = TYPE_BOOL;
		else OutValue.Type = TYPE_INT;
	}
	else if (Left.Type == TYPE_DOUBLE || Right.Type == TYPE_DOUBLE) {
		double LV = (Left.Type == TYPE_INT) ? (double)Left.IntValue : Left.DoubleValue;
		double RV = (Right.Type == TYPE_INT) ? (double)Right.IntValue : Right.DoubleValue;
		OutValue.DoubleValue = OperateDoubleValues(LV, RV, CurrBinary->Type, &IsBool);
		
		if (IsBool) OutValue.Type = TYPE_BOOL;
		else OutValue.Type = TYPE_DOUBLE;
	}
	
	

	return OutValue;
}

void ExecuteDeclaration(Expression* Expr, VariableEnvironment* Env) {
	DeclarationExpression* CurrDecl = Expr->Value.DeclExpr;
	Variable *CurrVariable = malloc(sizeof(Variable));
	if (CurrVariable == NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteDeclaration: Binary operation on non binary-accepted values." });
		return;
	}

	CurrVariable->VariableName = malloc(sizeof(char) * (strlen(CurrDecl->VarName.Value.stringVal)+1));
	if (CurrVariable->VariableName == NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteDeclaration: VarName malloc failed." });
		return;
	}
	strcpy(CurrVariable->VariableName, CurrDecl->VarName.Value.stringVal);
	
	//Check current and higher ranking environments for the same var name.
	//Implement it later

	CurrVariable->ForcedValueType = GetForcedVariableType(CurrDecl->VarType);

	CurrVariable->VariableValue = ExecuteExpression(CurrDecl->Value,Env);
	
	Variable* VarSearch = VarSearchEnvironment(CurrVariable->VariableName, Env);

	if (VarSearch != NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteDeclaration: Var with same name already exists." });
		return;
	}

	AddVariableToEnvironment(CurrVariable, Env);

	//PrintVariableEnvironment(Env);
}

void ExecuteAssignment(Expression* Expr, VariableEnvironment* Env) {
	AssignmentExpression* CurrAssign = Expr->Value.AssignExpr;
	Value VarName = ExecuteExpression(CurrAssign->VarName, Env);
	Value AssignValue = ExecuteExpression(CurrAssign->Value, Env);

	Variable* FoundVariable = VarSearchEnvironment(VarName.StringValue, Env);
	if (FoundVariable == NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteAssignment: Variable name doesnt exist." });
		return;
	}

	if (FoundVariable->ForcedValueType != -1 && (FoundVariable->ForcedValueType != AssignValue.Type)) {
		if (!((FoundVariable->ForcedValueType == TYPE_INT && AssignValue.Type == TYPE_DOUBLE) ||
			(FoundVariable->ForcedValueType == TYPE_DOUBLE && AssignValue.Type == TYPE_INT))) {
			PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteAssignment: Variable value isn't coherent." });
			return;
		}
	}

	AssignVariableValue(FoundVariable, AssignValue);

	return;
}

void ExecuteIf(Expression* Expr, VariableEnvironment* Env) {
	IfExpression* CurrIf = Expr->Value.IfExpr;
	Value ConditionResult = ExecuteExpression(CurrIf->Condition, Env);


	if (ConditionResult.Type != TYPE_BOOL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteIf: Condition is not a bool-return condition." });
		return;
	}
	
	if (ConditionResult.BoolValue == false) return;//Eventually check Else

	VariableEnvironment IfEnv = CreateEmptyEnvironment(Env);

	ExecuteExpression(CurrIf->IfBlock, &IfEnv);
}

void ExecuteWhile(Expression* Expr, VariableEnvironment* Env) {
	IfExpression* CurrWhile = Expr->Value.WhileExpr;
	Value ConditionResult = ExecuteExpression(CurrWhile->Condition, Env);


	if (ConditionResult.Type != TYPE_BOOL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteWhile: Condition is not a bool-return condition." });
		return;
	}

	if (ConditionResult.BoolValue == false) return;//Eventually check Else

	VariableEnvironment WhileEnv = CreateEmptyEnvironment(Env);

	while (ConditionResult.BoolValue == true) {
		ExecuteExpression(CurrWhile->IfBlock, &WhileEnv);
		ConditionResult = ExecuteExpression(CurrWhile->Condition, Env);
		//PrintVariableEnvironment(&WhileEnv); //This, for debug must be placed here: everytime it get's reset.
		WhileEnv = CreateEmptyEnvironment(Env);
	}
}

void ExecuteFunctionExpression(Expression* Expr, VariableEnvironment* Env) {
	FunctionExpression* CurrFuncExpr = Expr->Value.FuncExpr;
	Function* Func = malloc(sizeof(Function));

	if (Func == NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteFunctionExpression: Func malloc failed." });
		return;
	}

	Func->FuncEnvironment = CreateEmptyEnvironment(Env);
	Func->FunctionName = malloc(sizeof(char) * (strlen(CurrFuncExpr->FuncName.Value.stringVal)+1));
	if(Func->FunctionName==NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteFunctionExpression: FunctionName malloc failed." });
		return;
	}
	strcpy(Func->FunctionName, CurrFuncExpr->FuncName.Value.stringVal);


	Func->ReturnTypesN = CurrFuncExpr->ReturnTypesCount;
	Func->ReturnTypes = malloc(sizeof(FunctionReturnType) * Func->ReturnTypesN);
	if (Func->ReturnTypes == NULL) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteFunctionExpression: ReturnTypes malloc failed." });
		return;	
	}

	for (int i = 0; i < CurrFuncExpr->ReturnTypesCount; i++)
	{
		Func->ReturnTypes[i].Type = GetFuncReturnType(CurrFuncExpr->ReturnTypes[i], &(Func->ReturnTypes->StructCaseName), Env);
	}

	ExecuteExpression(CurrFuncExpr->Arguments, &Func->FuncEnvironment);
	Func->ArgumentsN = Func->FuncEnvironment.LastVarIndex;
	Func->ExpressionsBlock = CurrFuncExpr->FuncBlock;

	Func->LastOutValue = (Value){ TYPE_VOID, NULL };

	AddFunctionToEnvironment(Func, Env);
}

Value ExecuteExpression(Expression* Expr, VariableEnvironment* Env) {
	if(Expr==NULL)return (Value) { TYPE_VOID, NULL };

	switch (Expr->Type)
	{
	case EXPRESSION_NODE:
		printf("Executing Node->");
		return ExecuteNode(Expr, Env);
	case EXPRESSION_FACTOR:
		printf("Executing Factor->");
		return ExecuteFactor(Expr, Env);
	case EXPRESSION_TERM:
		printf("Executing Term->");
		return ExecuteTerm(Expr, Env);
	case EXPRESSION_BINARY:
		printf("Executing Binary->");
		return ExecuteBinary(Expr, Env);
	case EXPRESSION_DECLARATION:
		printf("Executing Declaration->");
		ExecuteDeclaration(Expr, Env);
		break;
	case EXPRESSION_ASSIGNMENT:
		printf("Executing Assignment->");
		ExecuteAssignment(Expr, Env);
		break;
	case EXPRESSION_IF:
		printf("Executing If->");
		ExecuteIf(Expr, Env);
		break;
	case EXPRESSION_WHILE:
		printf("Executing While->");
		ExecuteWhile(Expr, Env);
		break;
	case EXPRESSION_FUNC:
		printf("Executing Function Declaration->");
		ExecuteFunctionExpression(Expr, Env);
		break;
	default:
		printf("default");
		break;
	}

	return (Value){TYPE_VOID, NULL};
}

void Execute() {
	printf("\n");
	AdvanceExpression();
	GlobalEnvironment = CreateEmptyEnvironment(NULL);

	while (EndOfExpressions == false) {
		Value RetV = ExecuteExpression(CurrExpression, &GlobalEnvironment);
		printf("\n");
		if (RetV.Type != TYPE_VOID) {
			PrintValue(RetV);
			printf("\n");
		}
		AdvanceExpression();
	}

	PrintVariableEnvironment(&GlobalEnvironment);
}