#define _CRT_SECURE_NO_WARNINGS
#include "Interpreter.h"
#include "../Utils/PrintHelpers.h"

Expression* CurrExpression;
bool EndOfExpressions = false;
VariableEnvironment GlobalEnvironment;

//Print method from chatgpt. Later moved into PrintHelpers
void PrintValue(Value v) {
	switch (v.Type) {
	case TYPE_INT:
		printf("%d", v.IntValue);
		break;

	case TYPE_DOUBLE:
		printf("%f", v.DoubleValue);
		break;

	case TYPE_STRING:
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

void AdvanceExpression() {
	if (ExprFirst == NULL) {
		EndOfExpressions = true;
		return; //Add Errors
	}
	CurrExpression = ExprFirst->Expr;
	ExprFirst = ExprFirst->Next;
}

int OperateIntValues(int LValue, int RValue, BinaryExpressionType Operator) {
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
		return;
	}
}

double OperateDoubleValues(double LValue, double RValue, BinaryExpressionType Operator) {
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
		return;
	}
}

Value ExecuteNode(Expression* Expr){
	Node* CurrNode = Expr->Value.NodeExpr;
	if(CurrNode==NULL) PrintGrammarError((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNode: CurrNode is not a node." });
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
		CurrVal.StringValue = malloc(sizeof(char) * strlen(CurrNode->Value.Tok.Value.stringVal));
		if (CurrVal.StringValue == NULL) {
			PrintGrammarError((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: CurrVal malloc failed." });
			return;
		}

		strcpy(CurrVal.StringValue, CurrNode->Value.Tok.Value.stringVal);
		return CurrVal;

	case NODE_GROUPING:
		return ExecuteExpression(CurrNode->Value.NodeGrouping);

	//case NODE_BOOL:
	
	default:
		PrintGrammarError((GrammarError) { CurrNode->Value.Tok.Line, CurrNode->Value.Tok.EndColumn, "Error in ExecuteNdoe: Unknown node." });
		return;
	}

}

Value ExecuteFactor(Expression* Expr) {
	Factor* CurrFactor = Expr->Value.FactorExpr;
	Value FactValue = ExecuteExpression(CurrFactor->Value);
	
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

Value ExecuteTerm(Expression* Expr) {
	Term* CurrTerm = Expr->Value.TermExpr;
	Value Left = ExecuteExpression(CurrTerm->Left);
	Value Right = ExecuteExpression(CurrTerm->Right);
	Value OutValue;

	if ((Left.Type != TYPE_INT && Left.Type != TYPE_DOUBLE) || (Right.Type != TYPE_INT && Right.Type != TYPE_DOUBLE)) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteBinary: Binary operation on non binary-accepted values." });
		return;
	}

	if (CurrTerm->Type == TERM_NONE) return Left;

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

Value ExecuteBinary(Expression* Expr) {
	BinaryExpression* CurrBinary = Expr->Value.BinExpr;
	Value Left = ExecuteExpression(CurrBinary->Left);
	Value Right = ExecuteExpression(CurrBinary->Right);
	Value OutValue;


	if ((Left.Type != TYPE_INT && Left.Type != TYPE_DOUBLE) || (Right.Type != TYPE_INT && Right.Type != TYPE_DOUBLE)) {
		PrintGrammarError((GrammarError) { 0, 0, "Error in ExecuteBinary: Binary operation on non binary-accepted values." });
		return;
	}
	
	if (Left.Type == TYPE_INT && Right.Type == TYPE_INT) {
		OutValue.Type = TYPE_INT;
		OutValue.IntValue = OperateIntValues(Left.IntValue, Right.IntValue, CurrBinary->Type);
	}
	else if (Left.Type == TYPE_DOUBLE || Right.Type == TYPE_DOUBLE) {
		OutValue.Type = TYPE_DOUBLE;
		double LV = (Left.Type == TYPE_INT) ? (double)Left.IntValue : Left.DoubleValue;
		double RV = (Right.Type == TYPE_INT) ? (double)Right.IntValue : Right.DoubleValue;
		OutValue.DoubleValue = OperateDoubleValues(LV, RV, CurrBinary->Type);
	}
	
	

	return OutValue;
}

Value ExecuteExpression(Expression* Expr) {

	switch (Expr->Type)
	{
	case EXPRESSION_NODE:
		printf("Node:\n");
		return ExecuteNode(Expr);
	case EXPRESSION_FACTOR:
		printf("Factor:\n");
		return ExecuteFactor(Expr);
	case EXPRESSION_TERM:
		printf("Term:\n");
		return ExecuteTerm(Expr);
	case EXPRESSION_BINARY:
		printf("Binary:\n");
		return ExecuteBinary(Expr);
	default:
		printf("default");
		break;
	}

	return;
}

void Execute() {
	AdvanceExpression();
	GlobalEnvironment = (VariableEnvironment){ NULL, NULL };

	while (EndOfExpressions == false) {
		printf("\nCurrent Expression Type: %d\n", CurrExpression->Type);
		PrintValue(ExecuteExpression(CurrExpression));
		printf(" ");
		AdvanceExpression();
	}
}