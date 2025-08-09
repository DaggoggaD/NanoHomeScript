#define _CRT_SECURE_NO_WARNINGS
#include "Parser.h"

//Printing method. Only chatGpt part. Way too boring.
// Helper: restituisce la stringa dell'operatore binario
const char* binary_op_to_str(BinaryExpressionType t) {
	switch (t) {
	case BINARY_ADD: return "+";
	case BINARY_SUB: return "-";
	case BINARY_LESS: return "<";
	case BINARY_GREATER: return ">";
	case BINARY_LOE: return "<=";
	case BINARY_GOE: return ">=";
	case BINARY_EQUAL: return "==";
	case BINARY_NOE: return "!=";
	case BINARY_ASSIGN: return "=";
	case BINARY_AND: return "&&";
	case BINARY_OR: return "||";
	default: return "?";
	}
}

const char* term_op_to_str(TermType t) {
	switch (t) {
	case TERM_DIVISION: return "/";
	case TERM_MOLTIPLICATION: return "*";
	default: return "";
	}
}

const char* factor_op_to_str(FactorType f) {
	switch (f) {
	case FACTOR_NEGATIVE: return "-";
	case FACTOR_NOT: return "!";
	default: return "";
	}
}

static const char* decl_var_type_to_str(DeclarationVariableType t) {
	switch (t) {
	case VARIABLE_INT: return "int";
	case VARIABLE_DOUBLE: return "double";
	case VARIABLE_STRING: return "string";
	case VARIABLE_BOOL: return "bool";
	case VARIABLE_CUSTOM: return "custom";
	case VARIABLE_AUTO: return "auto";
	case VARIABLE_ARRAY: return "array";
	case VARIABLE_NONE: return "none";
	default: return "unknown";
	}
}

void PrintFunctionType(FunctionReturnInfo* retInfo) {
	if (retInfo == NULL) {
		printf("<null>");
		return;
	}

	switch (retInfo->Type) {
	case FUNCTION_INT:    printf("int"); break;
	case FUNCTION_DOUBLE: printf("double"); break;
	case FUNCTION_STRING: printf("string"); break;
	case FUNCTION_BOOL:   printf("bool"); break;
	case FUNCTION_ARRAY:  printf("array"); break;
	case FUNCTION_CUSTOM:
		if (retInfo->CustomTypeName != NULL)
			printf("%s", retInfo->CustomTypeName);
		else
			printf("<custom?>");
		break;
	case FUNCTION_VOID:   printf("void"); break;
	default:              printf("<unknown type>"); break;
	}
}

// Forward
void print_expression(Expression* expr);

// Stampa Node (primary)
void print_node(Node* node) {
	if (!node) {
		printf("<null node>");
		return;
	}
	switch (node->Type) {
	case NODE_NUMBER:
		if (node->Value.Tok.Type == INT) printf("%d", node->Value.Tok.Value.intVal);
		else printf("%lf", node->Value.Tok.Value.doubleVal);
		break;
	case NODE_STRING:
		printf("\"%s\"", node->Value.Tok.Value.stringVal);
		break;
		/*case NODE_BOOL:
		printf("%s", node->Value.Tok.boolean ? "true" : "false");
		break;*/
	case NODE_NULL:
		printf("null");
		break;
	case NODE_IDENTIFIER:
		printf("%s", node->Value.Tok.Value.stringVal);
		break;
	case NODE_CALL: {
		// nome funzione
		printf("%s(", node->Value.FuncCall.CallNameTok.Value.stringVal);
		for (size_t i = 0; i < (size_t)node->Value.FuncCall.ArgsNum; ++i) {
			print_expression(node->Value.FuncCall.Arguments[i]);
			if (i + 1 < (size_t)node->Value.FuncCall.ArgsNum)
				printf(", ");
		}
		printf(")");
		break;
	}
	case NODE_INDEX_ACCESS: {
		printf("%s[", node->Value.AtArrayIndex.ArrayNameTok.Value.stringVal);
		print_expression(node->Value.AtArrayIndex.Index);
		printf("]");
		break;
	}
	case NODE_GROUPING:
		print_expression(node->Value.NodeGrouping);
		break;
	case NODE_BLOCK:
		printf("(BLOCK: {");
		for (size_t i = 0; i < node->Value.Block.Size; i++) {
			print_expression(node->Value.Block.Expressions[i]);
			printf("; ");
		}
		printf("})");
		break;
	case NODE_RETURN:
		printf("(RETURN: ");
		if (node->Value.Return.ReturnNamesCount == 0) {
			printf("void");
		}
		else if (node->Value.Return.ReturnNamesCount == 1) {
			PrintFunctionType(node->Value.Return.ReturnNames[0]);
		}
		else {
			printf("(");
			for (int i = 0; i < node->Value.Return.ReturnNamesCount; i++) {
				PrintFunctionType(node->Value.Return.ReturnNames[i]);
				if (i < node->Value.Return.ReturnNamesCount - 1)
					printf(", ");
			}
			printf(")");
		}
		printf(")");
		break;
	default:
		printf("<unknown node>");
	}
}

void print_factor(Factor* factor) {
	if (!factor) {
		printf("<null factor>");
		return;
	}

	if (factor->Type != FACTOR_NONE) {
		printf("%s", factor_op_to_str(factor->Type));
	}
	print_expression(factor->Value);
}

void print_term(Term* term) {
	if (!term) {
		printf("<null term>");
		return;
	}

	// Left associativity: (left op right)
	printf("(");
	print_expression(term->Left);

	if (term->Type != TERM_NONE) {
		printf(" %s ", term_op_to_str(term->Type));
		print_expression(term->Right);
	}
	printf(")");
}

void print_binary_expr(BinaryExpression* bin) {
	if (!bin) {
		printf("<null binexpr>");
		return;
	}
	printf("(");
	print_expression(bin->Left);
	printf(" %s ", binary_op_to_str(bin->Type));
	print_expression(bin->Right);
	printf(")");
}

void print_decl_expr(DeclarationExpression* d) {
	printf("(");
	if (!d) {
		printf("<null decl>");
		return;
	}

	// keyword
	printf("VAR ");

	// wide: include tipo esplicito
	if (d->ExprType == DECLARATION_WITH_TYPE) {
		printf("%s", decl_var_type_to_str(d->VarType));
	}
	else printf("AUTO");

	// spazio e nome
	printf(" ");
	printf("%s", d->VarName.Value.stringVal);


	// initializer se presente
	if (d->Value) {
		printf(", ");
		print_expression(d->Value);
	}
	printf(")");
}

void print_assignment_expr(AssignmentExpression* a) {
	printf("(ASSIGN ");
	if (!a) {
		printf("<null assignment>");
		return;
	}

	// LHS: nome variabile (assumiamo IDENTIFIER)
	print_expression(a->VarName);
	printf(", ");

	// RHS
	if (a->Value) {
		print_expression(a->Value);
	}
	else {
		printf("<null value>");
	}
	printf(")");
}

void print_if_expression(IfExpression* a) {
	printf("(IF ");
	if (!a) {
		printf("<null assignment>");
		return;
	}
	print_expression(a->Condition);
	printf("{");
	print_expression(a->IfBlock);
	printf("}");
	printf(")");
}

void print_while_expression(WhileExpression* a) {
	printf("(While ");
	if (!a) {
		printf("<null assignment>");
		return;
	}
	print_expression(a->Condition);
	printf("{");
	print_expression(a->WhileBlock);
	printf("}");
	printf(")");
}

void print_function_expression(FunctionExpression* funcExpr) {
	if (funcExpr == NULL) {
		printf("<null function expression>\n");
		return;
	}

	printf("(Function: %s, Arguments: ", funcExpr->FuncName.Value.stringVal);

	print_expression(funcExpr->Arguments);

	printf(", returns: ");

	if (funcExpr->ReturnTypesCount == 0) {
		printf("void");
	}
	else if (funcExpr->ReturnTypesCount == 1) {
		PrintFunctionType(funcExpr->ReturnTypes[0]);
	}
	else {
		printf("(");
		for (int i = 0; i < funcExpr->ReturnTypesCount; i++) {
			PrintFunctionType(funcExpr->ReturnTypes[i]);
			if (i < funcExpr->ReturnTypesCount - 1)
				printf(", ");
		}
		printf(")");
	}

	printf(",\n");
	print_expression(funcExpr->FuncBlock);
	printf(");");
}

void print_expression(Expression* expr) {
	if (!expr) {
		printf("<null expr>");
		return;
	}

	switch (expr->Type) {
	case EXPRESSION_NODE:
		print_node(expr->Value.NodeExpr);
		break;
	case EXPRESSION_FACTOR:
		print_factor(expr->Value.FactorExpr);
		break;
	case EXPRESSION_TERM:
		print_term(expr->Value.TermExpr);
		break;
	case EXPRESSION_BINARY:
		print_binary_expr(expr->Value.BinExpr);
		break;
	case EXPRESSION_DECLARATION:
		print_decl_expr(expr->Value.DeclExpr);
		break;
	case EXPRESSION_ASSIGNMENT:
		print_assignment_expr(expr->Value.AssignExpr);
		break;
	case EXPRESSION_IF:
		print_if_expression(expr->Value.IfExpr);
		break;
	case EXPRESSION_WHILE:
		print_while_expression(expr->Value.WhileExpr);
		break;
	case EXPRESSION_FUNC:
		print_function_expression(expr->Value.FuncExpr);
		break;
	default:
		printf("<unknown expr kind>");
	}
}

//END OF PRINTING METHODS

DeclarationVariableType GetTokenDeclType() {
	switch (CurrToken.OpKwValue)
	{
	case KW_INT: return VARIABLE_INT;
	case KW_DOUBLE: return VARIABLE_DOUBLE;
	case KW_STRING: return VARIABLE_STRING;
	case KW_ARRAY: return VARIABLE_ARRAY;
		//case KWBOOL: return VARIABLE_BOOL;
	default:
		break;
	}
	return VARIABLE_NONE;
}


BinaryExpressionType GetBinaryExpressionType(TOKEN* Tok) {
	if (Tok == NULL) return BINARY_NONE;

	if (Tok->Type == OPERATOR) {
		switch (Tok->OpKwValue)
		{
		case SEP_OP_ADD: return BINARY_ADD;
		case SEP_OP_SUB: return BINARY_SUB;
		case SEP_OP_LESS: return BINARY_LESS;
		case SEP_OP_GREAT: return BINARY_GREATER;
		default: break;
		}
	}
	else {
		switch (Tok->OpKwValue)
		{
		case KW_COMPARE: return BINARY_EQUAL;
		case KW_COMPARE_INV: return BINARY_NOE;
		case KW_GOE: return BINARY_GOE;
		case KW_LOE: return BINARY_LOE;
		case KW_AND: return BINARY_AND;
		case KW_OR: return BINARY_OR;
		default: break;
		}
	}
	return BINARY_NONE;
}

FunctionType GetReturnType() {
	switch (CurrToken.OpKwValue)
	{
	case KW_INT: return FUNCTION_INT;
	case KW_DOUBLE: return FUNCTION_DOUBLE;
	case KW_STRING: return FUNCTION_STRING;
	case KW_VOID: return FUNCTION_VOID;
	case KW_ARRAY: return FUNCTION_ARRAY;
	//case KW_BOOL: return FUCNTION_BOOL;
	default: return FUNCTION_CUSTOM;
	}
}

void Advance() {
	CurrToken = NextToken;
	if (TokensFirst->next == NULL) return; //ADD ERROR PATTERNS

	TokensFirst = TokensFirst->next;
	if (TokensFirst->next == NULL) {
		ParserEndOfTokens = true;
	}
	else NextToken = TokensFirst->next->Tok;
}

Expression* MakeTokenNode(NodeType Type, TOKEN Tok) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTokenNode: CurrNode malloc failed." });
		return NULL;
	}

	CurrNode->Type = Type;
	CurrNode->Value.Tok = Tok;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTokenNode: CurrExpr malloc failed." });
		free(CurrNode);
		return NULL;
	}
	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}

//Could be included in maletokenNode, for clarity for now included here.
Expression* MakeGroupedNode(NodeType Type, Expression* Expr) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeGroupedNode: CurrNode malloc failed." });
		return NULL;
	}
	CurrNode->Type = Type;
	CurrNode->Value.NodeGrouping = Expr;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeGroupedNode: CurrExpr malloc failed." });
		free(CurrNode);
		return NULL;
	}

	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}

Expression* MakeNodeIndexAccess(NodeType Type, TOKEN NameToken, Expression* Index) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeNodeIndexAccess: CurrNode malloc failed." });
		return NULL;
	}
	CurrNode->Type = Type;
	CurrNode->Value.AtArrayIndex = (NodeIndexAccess){ NameToken, Index };

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeNodeIndexAccess: CurrExpr malloc failed." });
		free(CurrNode);
		return NULL;
	}

	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}


Expression* MakeFactor(FactorType Type, Expression* Left) {
	Factor* CurrFactor = malloc(sizeof(Factor));
	if (CurrFactor == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeFactor: CurrNode malloc failed." });
		return NULL;
	}

	CurrFactor->Type = Type;
	CurrFactor->Value = Left;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		free(CurrFactor);
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTokenNode: CurrExpr malloc failed." });
		return NULL;
	}
	CurrExpr->Type = EXPRESSION_FACTOR;
	CurrExpr->Value.FactorExpr = CurrFactor;

	return CurrExpr;
}

Expression* MakeTerm(TermType Type, Expression* Left, Expression* Right) {

	Term* CurrTerm = malloc(sizeof(Term));
	if (CurrTerm == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTerm: CurrNode malloc failed." });
		return NULL;
	}
	CurrTerm->Type = Type;
	CurrTerm->Left = Left;
	CurrTerm->Right = Right;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		free(CurrTerm);
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTerm: CurrExpr malloc failed." });
		return NULL; //ADD ERROR PATTERNS
	}
	CurrExpr->Type = EXPRESSION_TERM;
	CurrExpr->Value.TermExpr = CurrTerm;

	return CurrExpr;

}

Expression* MakeBinExpr(BinaryExpressionType Type, Expression* Left, Expression* Right) {
	BinaryExpression* CurrBinExpression = malloc(sizeof(BinaryExpression));
	if (CurrBinExpression == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeBinExpr: CurrNode malloc failed." });
		return NULL;
	}
	CurrBinExpression->Type = Type;
	CurrBinExpression->Left = Left;
	CurrBinExpression->Right = Right;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		free(CurrBinExpression);
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTokenNode: CurrExpr malloc failed." });
		return NULL; //ADD ERROR PATTERNS
	}CurrExpr->Type = EXPRESSION_BINARY;
	CurrExpr->Value.BinExpr = CurrBinExpression;

	return CurrExpr;
}

Expression* MakeDeclExpr(DeclarationExpressionType ExprType, DeclarationVariableType VarType, TOKEN VarName, Expression* Value) {
	DeclarationExpression* CurrDeclExpression = malloc(sizeof(DeclarationExpression));
	if (CurrDeclExpression == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeDeclExpr: CurrDeclExpression malloc failed." });
		return NULL;
	}

	CurrDeclExpression->ExprType = ExprType;
	CurrDeclExpression->VarType = VarType;
	CurrDeclExpression->VarName = VarName;
	CurrDeclExpression->Value = Value;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		free(CurrDeclExpression);
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeDeclExpr: CurrExpr malloc failed." });
		return NULL; //ADD ERROR PATTERNS
	}
	CurrExpr->Type = EXPRESSION_DECLARATION;
	CurrExpr->Value.DeclExpr = CurrDeclExpression;

	return CurrExpr;

}

Expression* MakeVarAssignment(Expression* VarName, Expression* Value) {
	AssignmentExpression* CurrAssignExpression = malloc(sizeof(AssignmentExpression));
	if (CurrAssignExpression == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeVarAssignment: CurrAssignExpression malloc failed." });
		return NULL;
	}

	CurrAssignExpression->VarName = VarName;
	CurrAssignExpression->Value = Value;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		free(CurrAssignExpression);
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeVarAssignment: CurrExpr malloc failed." });
		return NULL; //ADD ERROR PATTERNS
	}
	CurrExpr->Type = EXPRESSION_ASSIGNMENT;
	CurrExpr->Value.AssignExpr = CurrAssignExpression;

	return CurrExpr;
}

Expression* MakeBlockNode(NodeBlock Block) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeBlockNode: CurrNode malloc failed." });
		return NULL;
	}

	CurrNode->Type = NODE_BLOCK;
	CurrNode->Value.Block = Block;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeBlockNode: CurrExpr malloc failed." });
		return NULL; //ADD ERROR PATTERNS
	}
	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}

Expression* MakeIfExpression(Expression* Condition, Expression* IfBlock) {
	IfExpression* CurrIfExpr = malloc(sizeof(IfExpression));
	if (CurrIfExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeIfExpression: CurrIfExpr malloc failed." });
		return NULL;
	}

	CurrIfExpr->Condition = Condition;
	CurrIfExpr->IfBlock = IfBlock;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeIfExpression: CurrExpr malloc failed." });
		return NULL;
	}
	CurrExpr->Type = EXPRESSION_IF;
	CurrExpr->Value.IfExpr = CurrIfExpr;

	return CurrExpr;
}

//When if and while gets fused, (later in development) use MakeIfExpression.
Expression* MakeWhileExpression(Expression* Condition, Expression* WhileBlock) {
	WhileExpression* CurrWhileExpr = malloc(sizeof(WhileExpression));
	if (CurrWhileExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeWhileExpression: CurrWhileExpr malloc failed." });
		return NULL;
	}

	CurrWhileExpr->Condition = Condition;
	CurrWhileExpr->WhileBlock = WhileBlock;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeIfExpression: CurrExpr malloc failed." });
		return NULL;
	}
	CurrExpr->Type = EXPRESSION_WHILE;
	CurrExpr->Value.WhileExpr = CurrWhileExpr;

	return CurrExpr;
}

Expression* MakeFunctionExpression(FunctionExpression* FuncExpr){
	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeIfExpression: CurrExpr malloc failed." });
		return NULL;
	}
	CurrExpr->Type = EXPRESSION_FUNC;
	CurrExpr->Value.FuncExpr = FuncExpr;

	return CurrExpr;
}

Expression* MakeNodeReturn(FunctionReturnInfo** ReturnNames, int ReturnNamesCount) {
	NodeReturn Return = (NodeReturn){ ReturnNames, ReturnNamesCount };

	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeNodeReturn: CurrNode malloc failed." });
		return NULL;
	}

	CurrNode->Type = NODE_RETURN;
	CurrNode->Value.Return = Return;


	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeNodeReturn: CurrExpr malloc failed." });
		return NULL;
	}
	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}

FunctionReturnInfo** MakeTypeList(int* ReturnTypesCount, int Index) {
	FunctionReturnInfo** Info = malloc(sizeof(FunctionReturnInfo*));
	if (Info == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo malloc." });
		return NULL;
	}

	if (CurrToken.OpKwValue == SEP_LPAREN) {
		Advance();
		while (CurrToken.OpKwValue != SEP_RPAREN && (CurrToken.Type == KEYWORD || CurrToken.Type == IDENTIFIER)) {
			FunctionReturnInfo* CurrInfo = malloc(sizeof(FunctionReturnInfo));
			if (CurrInfo == NULL) {
				PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo malloc." });
				return NULL;
			}

			CurrInfo->Type = GetReturnType();
			

			if (CurrInfo->Type != FUNCTION_CUSTOM) CurrInfo->CustomTypeName = NULL;
			else {
				PrintToken(CurrToken);
				CurrInfo->CustomTypeName = malloc(sizeof(char) * strlen(CurrToken.Value.stringVal));
				if (CurrInfo->CustomTypeName == NULL) return NULL;
				strcpy(CurrInfo->CustomTypeName, CurrToken.Value.stringVal);
			}
			Info[Index] = CurrInfo;
			Index++;

			if (Index >= *ReturnTypesCount) {
				(*ReturnTypesCount)++;
				Info = realloc(Info, sizeof(FunctionReturnInfo*) * (*ReturnTypesCount));
				if (Info == NULL) {
					PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo realloc." });
					return NULL;
				}
			}

			Advance();
			if (CurrToken.OpKwValue == SEP_COMMA) Advance();
		}
		*ReturnTypesCount = Index;

		if (CurrToken.OpKwValue != SEP_RPAREN) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: missing ')' after return types declaration." });


	}
	else if (CurrToken.Type == KEYWORD || CurrToken.Type == IDENTIFIER) {
		Info[Index] = malloc(sizeof(FunctionReturnInfo));
		if (Info[Index] == NULL) {
			PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo malloc." });
			return NULL;
		}
		Info[Index]->Type = GetReturnType();

		if (Info[Index]->Type != FUNCTION_CUSTOM) Info[Index]->CustomTypeName = NULL;
		else {
			Info[Index]->CustomTypeName = malloc(sizeof(char) * strlen(CurrToken.Value.stringVal));
			if (Info[Index]->CustomTypeName == NULL) return NULL;
			strcpy(Info[Index]->CustomTypeName, CurrToken.Value.stringVal);
		}
	}
	else PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Missing return type in function expression" });

	return Info;
}



Expression* ParseBlock(int SepTokenStop) {
	NodeBlock Block;
	Block.Index = 0;
	Block.Size = 1; //change this and blocksize+=1 below to a higher value, to increase performance, possibly reducing memory optimization
	Block.Expressions = malloc(sizeof(Expression*) * Block.Size);
	if (Block.Expressions == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in ParseBlock: Block.Expressions malloc failed." });
		return NULL;
	}

	if (CurrToken.OpKwValue == SepTokenStop) {
		Block.Expressions == NULL;
		Block.Index = 0;
		Block.Size = 0;
	}

	while (CurrToken.OpKwValue != SepTokenStop && ParserEndOfTokens == false) {
		Expression* CurrExpr = ExpressionParse();

		if (Block.Index >= Block.Size) {
			Block.Size += 1;
			Block.Expressions = realloc(Block.Expressions, sizeof(Expression*) * Block.Size);
			if (Block.Expressions == NULL) {
				PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in ParseBlock: Block.Expressions malloc failed." });
				return NULL;
			}
		}

		Block.Expressions[Block.Index] = CurrExpr;
		Block.Index++;

		if (CurrToken.OpKwValue == SEP_SEMICOLON || CurrToken.OpKwValue == SEP_COMMA) Advance();
	}

	if (CurrToken.OpKwValue != SepTokenStop) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in ParseBlock: missing closing '{'." });
		free(Block.Expressions);
		return NULL;
	}

	Advance();

	return MakeBlockNode(Block);
}

Expression* ReturnNodeParse() {
	Advance();
	int RetNCount = 1;
	FunctionReturnInfo** ReturnList = MakeTypeList(&RetNCount, 0);
	Advance();

	return MakeNodeReturn(ReturnList, RetNCount);
}

Expression* NodeParse() {
	//CONVERT TO SWITCH

	if (CurrToken.Type == DOUBLE || CurrToken.Type == INT) {
		Expression* NumNode = MakeTokenNode(NODE_NUMBER, CurrToken);
		Advance();
		return NumNode;
	}

	else if (CurrToken.Type == STRING) {
		Expression* StringNode = MakeTokenNode(NODE_STRING, CurrToken);
		Advance();
		return StringNode;
	}

	else if (CurrToken.Type == IDENTIFIER) {
		TOKEN NameToken = CurrToken;
		Advance();

		//Function Call
		if (CurrToken.OpKwValue == SEP_LPAREN) {
			Advance();
			return NULL; //TO BE LATER IMPLEMENTED
		}
		else if (CurrToken.OpKwValue == SEP_LBRACKET) {
			Advance();
			Expression* Index = ExpressionParse();

			if (CurrToken.OpKwValue != SEP_RBRACKET) {
				PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Missing closing square parenthesis." });
				return NULL;
			}
			Advance();

			return MakeNodeIndexAccess(NODE_INDEX_ACCESS, NameToken, Index);
		}

		return MakeTokenNode(NODE_IDENTIFIER, NameToken);
	}

	else if (CurrToken.Type == KEYWORD) {
		switch (CurrToken.OpKwValue)
		{
		case KW_RETURN:
			return ReturnNodeParse();
		default:
			printf("\nENTERED DEFAULT IN NODE\n");
			return NULL;
		}
	}
	else if (CurrToken.OpKwValue == SEP_LPAREN) {
		Advance();
		Expression* Grouped = ExpressionParse();

		if (CurrToken.OpKwValue != SEP_RPAREN) {
			PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Missing closing parenthesis." });
			return NULL;
		}
		Advance();
		return MakeGroupedNode(NODE_GROUPING, Grouped);
	}

	else if (CurrToken.OpKwValue == SEP_LBRACE) {
		Advance();
		return ParseBlock(SEP_RBRACE);
	}

	PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Token/Series of Tokens isn't a NODE. Null return." });
	return NULL;
}

Expression* FactorParse() {
	if (CurrToken.OpKwValue == SEP_OP_NOT) {
		Advance();
		Expression* Left = FactorParse();
		return MakeFactor(FACTOR_NOT, Left);
	}
	else if (CurrToken.OpKwValue == SEP_OP_SUB) {
		Advance();
		Expression* Left = FactorParse();
		return MakeFactor(FACTOR_NEGATIVE, Left);
	}

	return NodeParse();
}

Expression* TermParse() {
	Expression* Left = FactorParse();

	while (CurrToken.OpKwValue == SEP_OP_MULT || CurrToken.OpKwValue == SEP_OP_DIV) {
		TermType Type;
		if (CurrToken.OpKwValue == SEP_OP_MULT) Type = TERM_MOLTIPLICATION;
		else Type = TERM_DIVISION;

		Advance();
		Expression* Right = FactorParse();
		Left = MakeTerm(Type, Left, Right);
	}
	return Left;
}

Expression* BinExprParse() {
	Expression* Left = TermParse();
	BinaryExpressionType Type = GetBinaryExpressionType(&CurrToken);

	while (Type != BINARY_NONE) {
		Advance();

		Expression* Right = TermParse();

		Left = MakeBinExpr(Type, Left, Right);
		Type = GetBinaryExpressionType(&CurrToken);
	}

	return Left;
}

Expression* DeclTypeExprParse() {
	Advance();
	DeclarationVariableType Type;

	if (CurrToken.Type == KEYWORD) {
		Type = GetTokenDeclType();
	}
	else if (CurrToken.Type == IDENTIFIER) {
		Type = VARIABLE_CUSTOM;
	}
	else {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in DeclTypeExprParse: Declaration TYPE not recognized." });
		return NULL;
	}

	Advance();
	TOKEN VarName = CurrToken;
	Advance();

	if (CurrToken.OpKwValue != SEP_EQUALS) {
		if (CurrToken.OpKwValue != SEP_SEMICOLON && CurrToken.OpKwValue != SEP_COMMA) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in DeclTypeExprParse: Missing ';'." });
		return MakeDeclExpr(DECLARATION_WITH_TYPE, Type, VarName, NULL);
	}

	Advance();

	Expression* Value = ExpressionParse();

	return MakeDeclExpr(DECLARATION_WITH_TYPE, Type, VarName, Value);
}

Expression* DeclAutoExprParse() {
	TOKEN VarName = CurrToken;
	Advance();

	if (CurrToken.OpKwValue != SEP_EQUALS) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in DeclAutoExprParse: Automatic type casting requires immediate assignment" });
	}

	Advance();

	Expression* Value = ExpressionParse();

	return MakeDeclExpr(DECLARATION_AUTO, VARIABLE_AUTO, VarName, Value);

	return NULL;
}

Expression* DeclExprParse() {
	Advance();
	if (CurrToken.OpKwValue == SEP_COLON) {
		return DeclTypeExprParse();
	}
	return DeclAutoExprParse();
}

//Change VarName back to an expression, and build LValueParse, to parse the left side of an expression, so that you can both use identifier and array access at the same time
//wich means you need to implement NODE_INDEX (array node access).
Expression* VarAssignmentParse() {
	Expression* VarName = NodeParse();
	Advance();

	Expression* Value = ExpressionParse();
	if (Value == NULL) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in VarAssignmentParse: Missing value expression" });
	return MakeVarAssignment(VarName, Value);
}

Expression* VarAssignmentArrayParse(Expression* VarName) {
	Advance();

	Expression* Value = ExpressionParse();
	if (Value == NULL) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in VarAssignmentParse: Missing value expression" });
	return MakeVarAssignment(VarName, Value);

}

Expression* IfWhileExpressionParse(int IfOrWhile) {
	Advance();
	Expression* Condition = ExpressionParse();
	if(Condition==NULL) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in IfExpressionParse: Missing condition expression" });
	
	if(CurrToken.OpKwValue!=SEP_LBRACE) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in IfExpressionParse: Missing '{' in if expression" });

	Expression* Block = ExpressionParse();

	if(IfOrWhile == IF_FUNCTION) return MakeIfExpression(Condition, Block);
	return MakeWhileExpression(Condition, Block);
}

Expression* FuncExpressionParse() {
	Advance();
	FunctionExpression* FuncExpression = malloc(sizeof(FunctionExpression));
	if(FuncExpression==NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FuncExpression malloc." });
		return NULL;
	}
	FuncExpression->ReturnTypesCount = 1;
	int Index = 0;

	FunctionReturnInfo** Info = malloc(sizeof(FunctionReturnInfo*));
	if (Info == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo malloc." });
		return NULL;
	}

	Info = MakeTypeList(&(FuncExpression->ReturnTypesCount), Index);
	Advance();
	
	FuncExpression->ReturnTypes = Info;

	if(CurrToken.Type != IDENTIFIER) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Missing function name." });

	FuncExpression->FuncName = CurrToken;
	Advance();

	if(CurrToken.OpKwValue != SEP_LPAREN) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Missing function's arguments '('." });
		
	Advance();
	FuncExpression->Arguments = ParseBlock(SEP_RPAREN);

	FuncExpression->FuncBlock = ExpressionParse();

	return MakeFunctionExpression(FuncExpression);
}

//REMEMBER: ARRAYS, FUNCTION CALLS, ETC NEED TO BE IMPLEMENTED LATER.
//That said: now, var:array name; and var name = {....} are fully dinamic.
//Array assignments, so name[expression] = ... are handled here (in the elif)
//Meanwhile, in situations like x = nameArray[expression], nameArray[expression] is a node: see grammar.
//Same for function calls: if you find nameCALL(...) without an output assignment, it'll also be handled in the elif.
//Again, if you instead find it in x = nameCALL(...) it'll be a node.
//Call() in a node is already initialized in NodeParse (only the main branch. there is nothing implemented yet). Arrays still needs to be implemented.
//Clear this comment when completed.
Expression* ExpressionParse() {
	//Start simple: add expceptions, like array[index] = 12; (wich is not identifier = expression;).
	//Handle Keyword starting expressions.
	if (CurrToken.Type == KEYWORD) {
		switch (CurrToken.OpKwValue) {
		case KW_VAR:
			return DeclExprParse();
		case KW_IF:
			return IfWhileExpressionParse(IF_FUNCTION);
		case KW_WHILE:
			return IfWhileExpressionParse(WHILE_FUNCTION);
		case KW_FUNC:
			return FuncExpressionParse();
		default:
			break;
		}
	}
	//Split this in, for example, IDENTIFIER "[" expr "]"... or IDENTIFIER "=" ...
	else if (CurrToken.Type == IDENTIFIER) {
		//Put these in a switch if they become too many
		switch (NextToken.OpKwValue)
		{
		case SEP_EQUALS:
			return VarAssignmentParse();
		case SEP_LBRACKET:
			return VarAssignmentArrayParse(NodeParse());


		default:
			break;
		}
	}

	return BinExprParse();
}

void Parse() {
	CurrToken = TokensFirst->Tok;
	if (TokensFirst != NULL) NextToken = TokensFirst->next->Tok;

	while (ParserEndOfTokens == false) {
		Expression* CurrExpr = ExpressionParse();
		print_expression(CurrExpr);
		printf("\n");

		if (CurrToken.OpKwValue == SEP_SEMICOLON) {
			Advance();
		}
		else if (CurrExpr == NULL) {
			PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in expression: Null return. Check function." });
			Advance();
		}
	}

}