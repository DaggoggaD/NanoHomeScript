#define _CRT_SECURE_NO_WARNINGS
#include "Interpreter.h"
#include "FreeInterpreter.h"
#include "InterpreterPrintHelper.h"
#include "InterpreterUtils.h"
#include "../Utils/PrintHelpers.h"


//========GLOBAL VARIABLES========

Expression* CurrExpression;
bool EndOfExpressions = false;
VariableEnvironment GlobalEnvironment;

//========EXECUTION METHODS========

void ExecutePrintCall(FunctionReturnInfo** ToPrint, int PrintN, VariableEnvironment* Env) {
	for (int i = 0; i < PrintN; i++)
	{
		Value CurrArgValue = ExecuteExpression(ToPrint[i]->Value, Env);

		if (CurrArgValue.Type == TYPE_IDENTIFIER) {
			Variable* Var = VarSearchEnvironment(CurrArgValue.StringValue, Env);
			if (Var != NULL) CurrArgValue = Var->VariableValue;
		}
		PrintOutValue(CurrArgValue);
	}
	printf("\n");
}

//Executes a function call, creating a new environment for it, assigning arguments and executing its block.
//If a return is found it returns it.
Value ExecuteFunctionCall(char* FuncName, FunctionReturnInfo** Args, int ArgsN, VariableEnvironment* Env) {
	Function* Func = FunctionSearchEnvironment(FuncName, Env);

	//Will be moved in a better place, with a switch and all built-in functions.
	//For debug purposes, here is fine.
	if (strcmp(FuncName, "print") == 0) {
		ExecutePrintCall(Args, ArgsN, Env);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

	if (Func == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteFunctionCall: No such named function." });
		FreeAll(&GlobalEnvironment);
		return (Value){ .Type = TYPE_VOID, NULL };
	}

	//Used to be sure that, if function is called multiple times, its environment is clean.
	VariableEnvironment SonEnvironment = CreateEmptyEnvironment(&Func->FuncEnvironment);

	if (ArgsN != Func->ArgumentsN) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteFunctionCall: Uncoherent number of argoments passed." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

	for (int i = 0; i < ArgsN; i++)
	{
		Value CurrArgValue = ExecuteExpression(Args[i]->Value, Env);

		if (CurrArgValue.Type == TYPE_IDENTIFIER) {
			Variable* Var = VarSearchEnvironment(CurrArgValue.StringValue, Env);
			if (Var != NULL) CurrArgValue = Var->VariableValue;
		}


		Func->FuncEnvironment.Variables[i].VariableValue = CurrArgValue;
	}

	
	//Return method is handled inside ExecuteNode (Block type).
	Value RetVal = ExecuteExpression(Func->ExpressionsBlock, &SonEnvironment);
	if (RetVal.ArrayValuesLastIndex == 1) return RetVal.ArrayValues[0];
	return RetVal;
}

//This method is called in ExecuteNode.
//Assigns a whole block's values to an array.
void ExecuteArrayAssignment(Value *Res, Value *ArrayValue, bool *IsArray) {
	//Array assignment
	
	if (Res->Type == TYPE_VOID || Res->Type == TYPE_RETURN) {
		*IsArray = false;
	}
	else if (*IsArray==true) {
		ArrayValue->ArrayValues[ArrayValue->ArrayValuesLastIndex] = *Res;
		ArrayValue->ArrayValuesLastIndex++;

		if (ArrayValue->ArrayValuesLastIndex >= ArrayValue->ArrayValuesSize) {
			ArrayValue->ArrayValuesSize+=10;
			Value* Temp = realloc(ArrayValue->ArrayValues, sizeof(Value) * ArrayValue->ArrayValuesSize);
			if (Temp == NULL) {
				PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in AddFunctionToEnvironment: Funcs realloc failed." });
				FreeAll(&GlobalEnvironment);
				return;
			}

			ArrayValue->ArrayValues = Temp;
		}

	}
}

//Executes a block node. If a return method is found, it returns it.
//Otherwise, it returns an array (if ExecuteArrayAssignment recognizes it as such). Returns void otherwise.
Value ExecuteBlockHelper(Node* CurrNode, VariableEnvironment* Env) {
	Value ArrayValue;
	bool IsArray = true;
	ArrayValue.ArrayValuesSize = 1;
	ArrayValue.ArrayValuesLastIndex = 0;
	ArrayValue.ArrayValues = malloc(sizeof(Value) * ArrayValue.ArrayValuesSize);

	if (ArrayValue.ArrayValues == NULL) {
		PrintGrammarWarning((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: ArrayValues malloc failed." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

	for (int i = 0; i < CurrNode->Value.Block.Index; i++)
	{
		//This for is quite confusing. Must change it later.
		//For now: 
		// If the expression is a return node, it executes it (so it gets it's value) and returns it.
		// This, though, means that if the return node is inside an if (or something else) inside a function,
		// it won't return aswell in the function (wich means that the if will terminate after the return, but the function will continue)
		// To fix this, i added the other if (Res.Type == TYPE_RETURN) below.
		// This way, since an if is an expression on it's own, if the if "returns" a value (in form of a return) it will return once again.
		// This is shitty, for now it works, but it MUST BE REDONE later.
		// For the other two lines:
		//  Value Res = ... executes the expression and stores it in Res, normally.
		//  ExecuteArrayAssignment(...) checks if Res is eligible to be an array value 
		//  (wich means it's a TYPE_VOID or a TYPE_RETURN, wich all ifs, whiles, functions etc are)


		if (CurrNode->Value.Block.Expressions[i]->Type == EXPRESSION_NODE && CurrNode->Value.Block.Expressions[i]->Value.NodeExpr->Type == NODE_RETURN) {
			return ExecuteExpression(CurrNode->Value.Block.Expressions[i], Env);
		}

		Value Res = ExecuteExpression(CurrNode->Value.Block.Expressions[i], Env);

		if (Res.Type == TYPE_RETURN) {
			return Res;
		}

		ExecuteArrayAssignment(&Res, &ArrayValue, &IsArray);
	}

	if (IsArray) {
		ArrayValue.Type = TYPE_ARRAY;
		return ArrayValue;
	}
	return (Value) { .Type = TYPE_VOID, NULL };
}

//Executes a return node. Called in ExecuteNode. Also handles multiple return values.
Value ExecuteReturnHelper(Node* CurrNode, VariableEnvironment* Env) {
	Value ReturnValue;

	ReturnValue.Type = TYPE_RETURN;
	ReturnValue.ArrayValuesSize = CurrNode->Value.Return.ReturnNamesCount + 1;
	ReturnValue.ArrayValuesLastIndex = CurrNode->Value.Return.ReturnNamesCount;
	ReturnValue.ArrayValues = malloc(sizeof(Value) * ReturnValue.ArrayValuesSize);

	if (ReturnValue.ArrayValues == NULL) {
		PrintGrammarWarning((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: ReturnValue malloc failed." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

	for (int i = 0; i < ReturnValue.ArrayValuesLastIndex; i++)
	{
		ReturnValue.ArrayValues[i] = ExecuteExpression((CurrNode->Value.Return.ReturnNames[i])->Value, Env);

		if (ReturnValue.ArrayValues[i].Type == TYPE_IDENTIFIER) {
			ReturnValue.ArrayValues[i] = VarSearchEnvironment(ReturnValue.ArrayValues[i].StringValue, Env)->VariableValue;
		}
	}

	return ReturnValue;
}

//Helper for ExecuteNode number case.
Value ExecuteNumberHelper(Node* CurrNode) {
	Value CurrVal;
	if (CurrNode->Value.Tok.Type == INT) {
		CurrVal.Type = TYPE_INT;
		CurrVal.IntValue = CurrNode->Value.Tok.Value.intVal;
		return CurrVal;
	}
	CurrVal.Type = TYPE_DOUBLE;
	CurrVal.DoubleValue = CurrNode->Value.Tok.Value.doubleVal;

	return CurrVal;
}

//Helper for ExecuteNode string case.
Value ExecuteStringHelper(Node* CurrNode) {
	Value CurrVal;
	CurrVal.Type = TYPE_STRING;
	CurrVal.StringValue = malloc(sizeof(char) * (strlen(CurrNode->Value.Tok.Value.stringVal) + 1));
	if (CurrVal.StringValue == NULL) {
		PrintGrammarWarning((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: CurrVal malloc failed." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

	strcpy(CurrVal.StringValue, CurrNode->Value.Tok.Value.stringVal);
	return CurrVal;
}

//Helper for ExecuteNode identifier case.
Value ExecuteIdentifierHelper(Node* CurrNode) {
	Value CurrVal;
	CurrVal.Type = TYPE_IDENTIFIER;
	CurrVal.StringValue = malloc(sizeof(char) * (strlen(CurrNode->Value.Tok.Value.stringVal) + 1));
	if (CurrVal.StringValue == NULL) {
		PrintGrammarWarning((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: CurrVal malloc failed." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

	strcpy(CurrVal.StringValue, CurrNode->Value.Tok.Value.stringVal);
	return CurrVal;
}

//Executes all node types. If the node expects a return value, it returns it.
Value ExecuteNode(Expression* Expr, VariableEnvironment *Env){
	Node* CurrNode = Expr->Value.NodeExpr;

	switch (CurrNode->Type)
	{
	case NODE_NUMBER:
		return ExecuteNumberHelper(CurrNode);

	case NODE_STRING:
		return ExecuteStringHelper(CurrNode);

	case NODE_GROUPING:
		return ExecuteExpression(CurrNode->Value.NodeGrouping, Env);
	
	case NODE_IDENTIFIER:
		return ExecuteIdentifierHelper(CurrNode);

	case NODE_BLOCK: 
		return ExecuteBlockHelper(CurrNode, Env);

	case NODE_CALL:
		return ExecuteFunctionCall(CurrNode->Value.FuncCall.CallNameTok.Value.stringVal, CurrNode->Value.FuncCall.Arguments, CurrNode->Value.FuncCall.ArgumentsNamesCount, Env);

	case NODE_RETURN:
		return ExecuteReturnHelper(CurrNode, Env);

	//case NODE_BOOL:

	default:
		PrintGrammarWarning((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: Unknown node." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
	}

}

//Executes Factors, inverting/negating values.
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
		if (FactValue.Type != TYPE_BOOL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteFactor: ! can't be performed on a non boolean." });
			FreeAll(&GlobalEnvironment);
			return (Value) { .Type = TYPE_VOID, NULL };
		}
		FactValue.BoolValue = !FactValue.BoolValue;
	}

	return FactValue;
}

//Executes terms, handling * and / operations. Checks for type coherence (must be improved).
Value ExecuteTerm(Expression* Expr, VariableEnvironment* Env) {
	Term* CurrTerm = Expr->Value.TermExpr;
	Value Left = ExecuteExpression(CurrTerm->Left, Env);
	Value Right = ExecuteExpression(CurrTerm->Right, Env);
	Value OutValue;

	if (CurrTerm->Type == TERM_NONE) return Left;

	if ((Left.Type != TYPE_INT && Left.Type != TYPE_DOUBLE) || (Right.Type != TYPE_INT && Right.Type != TYPE_DOUBLE)) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteBinary: Binary operation on non binary-accepted values." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
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

//Executes binary expressions, handling all binary operations. Checks for type coherence.
Value ExecuteBinary(Expression* Expr, VariableEnvironment* Env) {
	BinaryExpression* CurrBinary = Expr->Value.BinExpr;

	Value Left = ExecuteExpression(CurrBinary->Left, Env);
	Value Right = ExecuteExpression(CurrBinary->Right, Env);

	if (Left.Type == TYPE_IDENTIFIER) {
		Variable* Var = VarSearchEnvironment(Left.StringValue, Env);
		if (Var != NULL) Left = Var->VariableValue;
	}

	if (Right.Type == TYPE_IDENTIFIER) {
		Variable* Var = VarSearchEnvironment(Right.StringValue, Env);
		if (Var != NULL) Right = Var->VariableValue;
	}

	Value OutValue;

	if ((Left.Type != TYPE_INT && Left.Type != TYPE_DOUBLE) || (Right.Type != TYPE_INT && Right.Type != TYPE_DOUBLE)) {
		if (Left.Type == TYPE_STRING && Right.Type == TYPE_STRING) {

			if (CurrBinary->Type == BINARY_ADD) {
				return AddStringValues(CurrBinary->Type, &Left, &Right);
			}

			OutValue.Type = TYPE_BOOL;
			OutValue.BoolValue = OperateStringValues(Left.StringValue, Right.StringValue, CurrBinary->Type);
			return OutValue;
		}

		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteBinary: Binary operation on non binary-accepted values." });
		FreeAll(&GlobalEnvironment);
		return (Value) { .Type = TYPE_VOID, NULL };
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

//Executes variable declarations, adding them to the environment.
void ExecuteDeclaration(Expression* Expr, VariableEnvironment* Env) {
	DeclarationExpression* CurrDecl = Expr->Value.DeclExpr;
	Variable *CurrVariable = malloc(sizeof(Variable));
	if (CurrVariable == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteDeclaration: Binary operation on non binary-accepted values." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	CurrVariable->VariableName = malloc(sizeof(char) * (strlen(CurrDecl->VarName.Value.stringVal)+1));
	if (CurrVariable->VariableName == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteDeclaration: VarName malloc failed." });
		FreeAll(&GlobalEnvironment);
		return;
	}
	strcpy(CurrVariable->VariableName, CurrDecl->VarName.Value.stringVal);
	
	if(VarSearchEnvironment(CurrVariable->VariableName, Env)!=NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteDeclaration: Var with same name already exists." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	CurrVariable->ForcedValueType = GetForcedVariableType(CurrDecl->VarType);

	CurrVariable->VariableValue = ExecuteExpression(CurrDecl->Value,Env);
	
	Variable* VarSearch = VarSearchEnvironment(CurrVariable->VariableName, Env);

	if (VarSearch != NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteDeclaration: Var with same name already exists." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	AddVariableToEnvironment(CurrVariable, Env);

	//PrintVariableEnvironment(Env);
}

//Executes return assignments, checking for type coherence and multiple assignments.
void ExecuteReturnAssignment(AssignmentExpression* CurrAssign, VariableEnvironment* Env) {
	Node* CurrNode = CurrAssign->VarName->Value.NodeExpr;
	int ReturnNamesCount = CurrNode->Value.Return.ReturnNamesCount;
	Value AssignValue = ExecuteExpression(CurrAssign->Value, Env);

	for (int i = 0; i < ReturnNamesCount; i++)
	{
		Variable* FoundVariable = VarSearchEnvironment(CurrNode->Value.Return.ReturnNames[i]->Value->Value.NodeExpr->Value.Tok.Value.stringVal, Env);
		if (FoundVariable == NULL) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteAssignment: Variable name doesnt exist." });
			FreeAll(&GlobalEnvironment);
			return;
		}

		if (FoundVariable->ForcedValueType != -1 && (FoundVariable->ForcedValueType != AssignValue.ArrayValues[i].Type)) {
			if (!((FoundVariable->ForcedValueType == TYPE_INT && AssignValue.ArrayValues[i].Type == TYPE_DOUBLE) ||
				(FoundVariable->ForcedValueType == TYPE_DOUBLE && AssignValue.ArrayValues[i].Type == TYPE_INT))) {
				PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteAssignment: Variable value isn't coherent." });
				FreeAll(&GlobalEnvironment);
				return;
			}
		}

		AssignVariableValue(FoundVariable, AssignValue.ArrayValues[i]);
	}
}

//Executes assignments, checking for type coherence and multiple assignments.
void ExecuteAssignment(Expression* Expr, VariableEnvironment* Env) {
	AssignmentExpression* CurrAssign = Expr->Value.AssignExpr;
	if (CurrAssign->VarName->Type == EXPRESSION_NODE && CurrAssign->VarName->Value.NodeExpr->Type == NODE_RETURN) {
		ExecuteReturnAssignment(CurrAssign, Env);
		return;
	}

	Value VarName = ExecuteExpression(CurrAssign->VarName, Env);
	Value AssignValue = ExecuteExpression(CurrAssign->Value, Env);

	if (AssignValue.Type == TYPE_IDENTIFIER) {
		Variable* Var = VarSearchEnvironment(AssignValue.StringValue, Env);
		if (Var != NULL) AssignValue = Var->VariableValue;
	}

	Variable* FoundVariable = VarSearchEnvironment(VarName.StringValue, Env);
	if (FoundVariable == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteAssignment: Variable name doesnt exist." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	if (FoundVariable->ForcedValueType != -1 && (FoundVariable->ForcedValueType != AssignValue.Type)) {
		if (!((FoundVariable->ForcedValueType == TYPE_INT && AssignValue.Type == TYPE_DOUBLE) ||
			(FoundVariable->ForcedValueType == TYPE_DOUBLE && AssignValue.Type == TYPE_INT))) {
			PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteAssignment: Variable value isn't coherent." });
			FreeAll(&GlobalEnvironment);
			return;
		}
	}

	AssignVariableValue(FoundVariable, AssignValue);

	return;
}

//Executes if expressions. Also creates a new environment for the if block.
Value ExecuteIf(Expression* Expr, VariableEnvironment* Env) {
	IfExpression* CurrIf = Expr->Value.IfExpr;
	Value ConditionResult = ExecuteExpression(CurrIf->Condition, Env);


	if (ConditionResult.Type != TYPE_BOOL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteIf: Condition is not a bool-return condition." });
		FreeAll(&GlobalEnvironment);
		return;
	}
	
	if (ConditionResult.BoolValue == false) return (Value) { TYPE_VOID, NULL };//Eventually check Else

	VariableEnvironment IfEnv = CreateEmptyEnvironment(Env);

	Value Val = ExecuteExpression(CurrIf->IfBlock, &IfEnv);
	if(Val.Type == TYPE_RETURN)
	{
		return Val;
	}
	return (Value) { TYPE_VOID, NULL };
}

//Executes while expressions. Also creates a new environment for the while block.
Value ExecuteWhile(Expression* Expr, VariableEnvironment* Env) {
	IfExpression* CurrWhile = Expr->Value.WhileExpr;
	Value ConditionResult = ExecuteExpression(CurrWhile->Condition, Env);


	if (ConditionResult.Type != TYPE_BOOL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteWhile: Condition is not a bool-return condition." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	if (ConditionResult.BoolValue == false) return (Value) { TYPE_VOID, NULL };

	VariableEnvironment WhileEnv = CreateEmptyEnvironment(Env);
	Value LastVal = (Value){ TYPE_VOID, NULL };

	while (ConditionResult.BoolValue == true) {
		LastVal = ExecuteExpression(CurrWhile->IfBlock, &WhileEnv);

		ConditionResult = ExecuteExpression(CurrWhile->Condition, Env);
		//PrintVariableEnvironment(&WhileEnv); //This, for debug must be placed here: everytime it get's reset.
		WhileEnv = CreateEmptyEnvironment(Env);

		if (LastVal.Type == TYPE_RETURN)
		{
			return LastVal;
		}
		LastVal = (Value){ TYPE_VOID, NULL };
	}

	return LastVal;
}

//Executes function expressions (declarations), creating a new function and adding it to the environment.
void ExecuteFunctionExpression(Expression* Expr, VariableEnvironment* Env) {
	FunctionExpression* CurrFuncExpr = Expr->Value.FuncExpr;
	Function* Func = malloc(sizeof(Function));

	if (Func == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteFunctionExpression: Func malloc failed." });
		FreeAll(&GlobalEnvironment);
		return;
	}

	Func->FuncEnvironment = CreateEmptyEnvironment(Env);
	Func->FunctionName = malloc(sizeof(char) * (strlen(CurrFuncExpr->FuncName.Value.stringVal)+1));
	if(Func->FunctionName==NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteFunctionExpression: FunctionName malloc failed." });
		FreeAll(&GlobalEnvironment);
		return;
	}
	strcpy(Func->FunctionName, CurrFuncExpr->FuncName.Value.stringVal);


	Func->ReturnTypesN = CurrFuncExpr->ReturnTypesCount;
	Func->ReturnTypes = malloc(sizeof(FunctionReturnType) * Func->ReturnTypesN);
	if (Func->ReturnTypes == NULL) {
		PrintInterpreterError((GrammarError) { CurrExpression->Line, 0, "Error in ExecuteFunctionExpression: ReturnTypes malloc failed." });
		FreeAll(&GlobalEnvironment);
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

//Main execution method. Calls the right execution method based on expression type.
Value ExecuteExpression(Expression* Expr, VariableEnvironment* Env) {
	if(Expr==NULL)return (Value) { TYPE_VOID, NULL };
	

	switch (Expr->Type)
	{
	case EXPRESSION_NODE:
		//printf("Executing Node->");
		return ExecuteNode(Expr, Env);
	case EXPRESSION_FACTOR:
		//printf("Executing Factor->");
		return ExecuteFactor(Expr, Env);
	case EXPRESSION_TERM:
		//printf("Executing Term->");
		return ExecuteTerm(Expr, Env);
	case EXPRESSION_BINARY:
		//printf("Executing Binary->");
		return ExecuteBinary(Expr, Env);
	case EXPRESSION_DECLARATION:
		//printf("Executing Declaration->");
		ExecuteDeclaration(Expr, Env);
		break;
	case EXPRESSION_ASSIGNMENT:
		//printf("Executing Assignment->");
		ExecuteAssignment(Expr, Env);
		break;
	case EXPRESSION_IF:
		//printf("Executing If->");
		return ExecuteIf(Expr, Env);
	case EXPRESSION_WHILE:
		//printf("Executing While->");
		return ExecuteWhile(Expr, Env);
	case EXPRESSION_FUNC:
		//printf("Executing Function Declaration->");
		ExecuteFunctionExpression(Expr, Env);
		break;
	default:
		//printf("default");
		break;
	}
	
	return (Value){TYPE_VOID, NULL};
}

//Main execution loop. Advances through expressions, executing them one by one.
void Execute() {
	
	AdvanceExpression();
	GlobalEnvironment = CreateEmptyEnvironment(NULL);

	while (EndOfExpressions == false) {
		Value RetV = ExecuteExpression(CurrExpression, &GlobalEnvironment);
		AdvanceExpression();
	}

	//PrintVariableEnvironment(&GlobalEnvironment);
}