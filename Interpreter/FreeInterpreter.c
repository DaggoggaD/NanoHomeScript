#include "FreeInterpreter.h"

//Frees a value.
//NOTE: it does NOT free the Value itself, just its contents.
void FreeValue(Value* Val) {
	if (Val->StringValue != NULL) {
		free(Val->StringValue);
		Val->StringValue = NULL;
	}
	else if (Val->ArrayValues != NULL) {
		for (int i = 0; i < Val->ArrayValuesLastIndex; i++)
		{
			FreeValue(&Val->ArrayValues[i]);
		}
		free(Val->ArrayValues);
		Val->ArrayValues = NULL;
		Val->ArrayValuesLastIndex = 0;
		Val->ArrayValuesSize = 0;
	}
	return;
}

void FreeReturnType(FunctionReturnType* RetType) {
	FreeValue(&RetType->StructCaseName);
	return;
}

//Frees a variable.
//NOTE: it does NOT free the Variable itself, just its contents.
void FreeVariable(Variable* Var) {
	if (Var->VariableName != NULL) {
		free(Var->VariableName);
		Var->VariableName = NULL;
	}
	FreeValue(&(Var->VariableValue));
	return;
}

//Frees a function.
//NOTE: it does NOT free the Function itself, just its contents.
void FreeFunction(Function* Func) {
	if (Func->FunctionName != NULL) {
		free(Func->FunctionName);
		Func->FunctionName = NULL;
	}

	for (int i = 0; i < Func->ReturnTypesN; i++)
	{
		FreeReturnType(&Func->ReturnTypes[i]);
	}
	free(Func->ReturnTypes);
	Func->ReturnTypes = NULL;
	Func->ReturnTypesN = 0;

	FreeEnvironment(&Func->FuncEnvironment, false);

	free(Func->ExpressionsBlock);
	Func->ExpressionsBlock = NULL;

	FreeValue(&Func->LastOutValue);
}

//Frees an environment, including its parents.
//NOTE: it does NOT free the Environment itself, just its contents.
//NOTE: frees parents, but doesn't check if they are referenced elsewhere.
void FreeEnvironment(VariableEnvironment* Env, bool AlsoParents) {
	if (Env->Variables != NULL) {
		for (int i = 0; i < Env->LastVarIndex; i++)
		{
			FreeVariable(&Env->Variables[i]);
		}
		free(Env->Variables);
		Env->Variables = NULL;
	}
	Env->LastVarIndex = 0;
	Env->VariablesSize = 0;

	if (Env->Functions != NULL) {
		for (int i = 0; i < Env->LastFuncIndex; i++)
		{
			FreeFunction(&Env->Functions[i]);
		}
		free(Env->Functions);
		Env->Functions = NULL;
	}
	Env->LastFuncIndex = 0;
	Env->FunctionsSize = 0;


	if (AlsoParents && Env->ParentEnvironment != NULL) {
		FreeEnvironment(Env->ParentEnvironment, true);
		Env->ParentEnvironment = NULL;
	}
}

//Emergency garbage collector, frees everything.
void FreeAll(VariableEnvironment* Env) {
	FreeEnvironment(Env, true);
}