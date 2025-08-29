#define _CRT_SECURE_NO_WARNINGS
#include "Parser.h"
#include "../Utils/PrintHelpers.h"

//Utility Variables
TOKEN CurrToken;
TOKEN NextToken;
TOKEN TwoTokensAhead;
bool ParserEndOfTokens = false;

//========PARSER UTILITIES========

DeclarationVariableType GetTokenDeclType() {
	switch (CurrToken.OpKwValue)
	{
	case KW_INT: return VARIABLE_INT;
	case KW_DOUBLE: return VARIABLE_DOUBLE;
	case KW_STRING: return VARIABLE_STRING;
	case KW_ARRAY: return VARIABLE_ARRAY;
		//case KWBOOL: return VARIABLE_BOOL;
	default: break;
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
		case SEP_OP_MOD: return BINARY_MOD;
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

void AddExpression(Expression* Expr) {
	ExpressionList* Curr = malloc(sizeof(ExpressionList));
	if (Curr == NULL){
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in AddExpression: CurrExpr malloc failed." });
		return NULL;
	}

	Curr->Expr = Expr;
	Curr->Next = NULL;
	
	if (ExprFirst == NULL) {
		ExprFirst = Curr;
		ExprLast = Curr;
		return;
	}

	ExprLast->Next = Curr;
	ExprLast = Curr;
	return;
}

void Advance() {
	CurrToken = NextToken;
	if (TokensFirst->next == NULL) return; //ADD ERROR PATTERNS

	TokensFirst = TokensFirst->next;
	if (TokensFirst->next == NULL) {
		ParserEndOfTokens = true;
	}
	else {
		NextToken = TokensFirst->next->Tok;

		if (TokensFirst->next->next == NULL) {
			TwoTokensAhead = (TOKEN){ 0 };
			return;
		}
		else TwoTokensAhead = TokensFirst->next->next->Tok;
	}
}

//========EXPRESSION INTIALIZATIONS HELPERS========

Expression* MakeExpression(ExpressionValue ExprVal, ExpressionType Type){
	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeNodeIndexAccess: CurrExpr malloc failed." });
		return NULL;
	}

	CurrExpr->Type = Type;
	CurrExpr->Value = ExprVal;
	return CurrExpr;
}

Expression* MakeTokenNode(NodeType Type, TOKEN Tok) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeTokenNode: CurrNode malloc failed." });
		return NULL;
	}
	CurrNode->Type = Type;
	CurrNode->Value.Tok = Tok;

	ExpressionValue Value;
	Value.NodeExpr = CurrNode;
	return MakeExpression(Value, EXPRESSION_NODE);
}

Expression* MakeGroupedNode(NodeType Type, Expression* Expr) {
	//Could be included in maletokenNode, for clarity for now included here.
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeGroupedNode: CurrNode malloc failed." });
		return NULL;
	}
	CurrNode->Type = Type;
	CurrNode->Value.NodeGrouping = Expr;

	ExpressionValue Value;
	Value.NodeExpr = CurrNode;

	return MakeExpression(Value, EXPRESSION_NODE);
}

Expression* MakeNodeIndexAccess(NodeType Type, TOKEN NameToken, Expression* Index) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeNodeIndexAccess: CurrNode malloc failed." });
		return NULL;
	}
	CurrNode->Type = Type;
	CurrNode->Value.AtArrayIndex = (NodeIndexAccess){ NameToken, Index };

	ExpressionValue Value;
	Value.NodeExpr = CurrNode;

	return MakeExpression(Value, EXPRESSION_NODE);
}

Expression* MakeFactor(FactorType Type, Expression* Left) {
	Factor* CurrFactor = malloc(sizeof(Factor));
	if (CurrFactor == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeFactor: CurrNode malloc failed." });
		return NULL;
	}

	CurrFactor->Type = Type;
	CurrFactor->Value = Left;

	ExpressionValue Value;
	Value.FactorExpr = CurrFactor;

	return MakeExpression(Value, EXPRESSION_FACTOR);
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

	ExpressionValue Value;
	Value.TermExpr = CurrTerm;

	return MakeExpression(Value, EXPRESSION_TERM);
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

	ExpressionValue Value;
	Value.BinExpr = CurrBinExpression;

	return MakeExpression(Value, EXPRESSION_BINARY);
}

Expression* MakeDeclExpr(DeclarationVariableType VarType, TOKEN VarName, Expression* Value) {
	DeclarationExpression* CurrDeclExpression = malloc(sizeof(DeclarationExpression));
	if (CurrDeclExpression == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeDeclExpr: CurrDeclExpression malloc failed." });
		return NULL;
	}

	CurrDeclExpression->VarType = VarType;
	CurrDeclExpression->VarName = VarName;
	CurrDeclExpression->Value = Value;

	ExpressionValue ExprValue;
	ExprValue.DeclExpr = CurrDeclExpression;

	return MakeExpression(ExprValue, EXPRESSION_DECLARATION);
}

Expression* MakeVarAssignment(Expression* VarName, Expression* Value) {
	AssignmentExpression* CurrAssignExpression = malloc(sizeof(AssignmentExpression));
	if (CurrAssignExpression == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeVarAssignment: CurrAssignExpression malloc failed." });
		return NULL;
	}

	CurrAssignExpression->VarName = VarName;
	CurrAssignExpression->Value = Value;

	ExpressionValue ExprValue;
	ExprValue.AssignExpr = CurrAssignExpression;

	return MakeExpression(ExprValue, EXPRESSION_ASSIGNMENT);
}

Expression* MakeBlockNode(NodeBlock Block) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeBlockNode: CurrNode malloc failed." });
		return NULL;
	}

	CurrNode->Type = NODE_BLOCK;
	CurrNode->Value.Block = Block;

	ExpressionValue ExprValue;
	ExprValue.NodeExpr = CurrNode;

	return MakeExpression(ExprValue, EXPRESSION_NODE);
}

Expression* MakeCallExpression(NodeCall ArgsList, int ArgsN) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeCallExpression: CurrNode malloc failed." });
		return NULL;
	}
	CurrNode->Type = NODE_CALL;
	CurrNode->Value.FuncCall = ArgsList;

	ExpressionValue ExprValue;
	ExprValue.NodeExpr = CurrNode;

	return MakeExpression(ExprValue, EXPRESSION_NODE);
}

Expression* MakeIfExpression(Expression* Condition, Expression* IfBlock) {
	IfExpression* CurrIfExpr = malloc(sizeof(IfExpression));
	if (CurrIfExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeIfExpression: CurrIfExpr malloc failed." });
		return NULL;
	}

	CurrIfExpr->Condition = Condition;
	CurrIfExpr->IfBlock = IfBlock;

	ExpressionValue ExprValue;
	ExprValue.IfExpr = CurrIfExpr;

	return MakeExpression(ExprValue, EXPRESSION_IF);
}

Expression* MakeWhileExpression(Expression* Condition, Expression* WhileBlock) {
	//When if and while gets fused, (later in development) use MakeIfExpression.
	WhileExpression* CurrWhileExpr = malloc(sizeof(WhileExpression));
	if (CurrWhileExpr == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in MakeWhileExpression: CurrWhileExpr malloc failed." });
		return NULL;
	}

	CurrWhileExpr->Condition = Condition;
	CurrWhileExpr->WhileBlock = WhileBlock;

	ExpressionValue ExprValue;
	ExprValue.WhileExpr = CurrWhileExpr;

	return MakeExpression(ExprValue, EXPRESSION_WHILE);
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

	ExpressionValue ExprValue;
	ExprValue.NodeExpr = CurrNode;

	return MakeExpression(ExprValue, EXPRESSION_NODE);
}

FunctionReturnInfo** MakeTypeList(int* ReturnTypesCount, int Index) {
	FunctionReturnInfo** Info = malloc(sizeof(FunctionReturnInfo*));
	if (Info == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo malloc." });
		return NULL;
	}

	if (CurrToken.OpKwValue == SEP_LPAREN) {
		Advance();
		while (CurrToken.Type != OPERATOR || CurrToken.OpKwValue == SEP_OP_SUB ||CurrToken.OpKwValue == SEP_LBRACE || CurrToken.OpKwValue == SEP_LPAREN) { //SEP_LBRACE is for array values immidiate insert. Could be done more elegantly.
			FunctionReturnInfo* CurrInfo = malloc(sizeof(FunctionReturnInfo));
			if (CurrInfo == NULL) {
				PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Failed FunctionReturnInfo malloc." });
				return NULL;
			}

			CurrInfo->Type = GetReturnType();

			if (CurrToken.Type == KEYWORD) {
				CurrInfo->Value = NodeParse();
			}
			else {
				CurrInfo->Value = ExpressionParse();
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

			if(CurrToken.OpKwValue != SEP_RPAREN) Advance();
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
		Info[Index]->Value = NodeParse();
	}
	else PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in FuncExpressionParse: Missing return type in function expression" });

	return Info;
}

//========EXPRESSION PARSE FUNCTIONS========

Expression* BlockParse(int SepTokenStop) {
	NodeBlock Block;
	Block.Index = 0;
	Block.Size = 1; //change this and blocksize+=1 below to a higher value, to increase performance, possibly reducing memory optimization
	Block.Expressions = malloc(sizeof(Expression*) * Block.Size);
	if (Block.Expressions == NULL) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in BlockParse: Block.Expressions malloc failed." });
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
				PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in BlockParse: Block.Expressions malloc failed." });
				return NULL;
			}
		}

		Block.Expressions[Block.Index] = CurrExpr;
		Block.Index++;

		if (CurrToken.OpKwValue == SEP_SEMICOLON || CurrToken.OpKwValue == SEP_COMMA) Advance();
	}

	if (CurrToken.OpKwValue != SepTokenStop) {
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in BlockParse: missing closing '{'." });
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
	switch (CurrToken.Type)
	{
	case DOUBLE:
	case INT: {
		Expression* NumNode = MakeTokenNode(NODE_NUMBER, CurrToken);
		Advance();
		return NumNode;
	}
	
	case STRING: {
		Expression* StringNode = MakeTokenNode(NODE_STRING, CurrToken);
		Advance();
		return StringNode;
	}
	
	case IDENTIFIER: {
		TOKEN NameToken = CurrToken;
		Advance();

		//Function Call
		if (CurrToken.OpKwValue == SEP_LPAREN) {
			int RetNCount = 1;
			FunctionReturnInfo** ArgumentsList = MakeTypeList(&RetNCount, 0);

			NodeCall Call = (NodeCall){ NameToken, ArgumentsList, RetNCount };
			Advance();

			return MakeCallExpression(Call, RetNCount);
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
	
	case KEYWORD: {
		switch (CurrToken.OpKwValue)
		{
		case KW_RETURN:
			return ReturnNodeParse();
		default:
			return NULL;
		}
	}
	
	case OPERATOR: {
		if (CurrToken.OpKwValue == SEP_LPAREN) {
			//handle return assignment
			//after the if NEEDS TO BE moved in var assignment parse.
			if (TwoTokensAhead.OpKwValue == SEP_COMMA) {
				Expression* ReturnExpr = malloc(sizeof(Expression));
				if( ReturnExpr == NULL) {
					PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Failed Expression malloc." });
					return NULL;
				}
				ReturnExpr->Type = EXPRESSION_NODE;

				Node* ReturnNode = malloc(sizeof(Node));
				if( ReturnNode == NULL) {
					PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Failed Node malloc." });
					return NULL;
				}
				ReturnNode->Type = NODE_RETURN;


				NodeReturn Ret;
				Ret.ReturnNamesCount = 1;

				FunctionReturnInfo** Info = malloc(sizeof(FunctionReturnInfo*));
				if (Info == NULL) {
					PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Failed FunctionReturnInfo malloc." });
					return NULL;
				}

				Info = MakeTypeList(&Ret.ReturnNamesCount, 0);
				Advance();
				Ret.ReturnNames = Info;

				ReturnNode->Value.Return = Ret;
				ReturnExpr->Value.NodeExpr = ReturnNode;


				Expression* VarName = ReturnExpr;
				Advance();
				Expression* Value = ExpressionParse();
				if (Value == NULL) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in VarAssignmentParse: Missing value expression" });
				Advance();
				return MakeVarAssignment(VarName, Value);
			}
			//END OF NEEDS TO BE REDONE
			
			
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
			return BlockParse(SEP_RBRACE);
		}
	}

	default:
		PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in NodeParse: Token/Series of Tokens isn't a NODE. Null return." });
		return NULL;
	}
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
		//Added Rparen for function arguments. Remove that if it causes problems. Extensive tests needed.
		if (CurrToken.OpKwValue != SEP_SEMICOLON && CurrToken.OpKwValue != SEP_COMMA && CurrToken.OpKwValue!=SEP_RPAREN) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in DeclTypeExprParse: Missing ';'." });
		return MakeDeclExpr(Type, VarName, NULL);
	}

	Advance();

	Expression* Value = ExpressionParse();

	return MakeDeclExpr(Type, VarName, Value);
}

Expression* DeclAutoExprParse() {
	TOKEN VarName = CurrToken;
	Advance();

	if (CurrToken.OpKwValue != SEP_EQUALS) PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in DeclAutoExprParse: Automatic type casting requires immediate assignment" });

	Advance();

	Expression* Value = ExpressionParse();

	return MakeDeclExpr(VARIABLE_AUTO, VarName, Value);

	return NULL;
}

Expression* DeclExprParse() {
	Advance();
	if (CurrToken.OpKwValue == SEP_COLON) {
		return DeclTypeExprParse();
	}
	return DeclAutoExprParse();
}

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
	FuncExpression->Arguments = BlockParse(SEP_RPAREN);

	FuncExpression->FuncBlock = ExpressionParse();

	return MakeFunctionExpression(FuncExpression);
}

Expression* ExpressionParse() {
	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) {
			PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in ExpressionParse: Failed Expression malloc." });
			return NULL;
	}

	if (CurrToken.Type == KEYWORD) {
		switch (CurrToken.OpKwValue) {
		case KW_VAR:
			CurrExpr = DeclExprParse();
			CurrExpr->Line = CurrToken.Line;
			return CurrExpr;
		case KW_IF:
			CurrExpr = IfWhileExpressionParse(IF_FUNCTION);
			CurrExpr->Line = CurrToken.Line;
			return CurrExpr;
		case KW_WHILE:
			CurrExpr = IfWhileExpressionParse(WHILE_FUNCTION);
			CurrExpr->Line = CurrToken.Line;
			return CurrExpr;
		case KW_FUNC:
			CurrExpr = FuncExpressionParse();
			CurrExpr->Line = CurrToken.Line;
			return CurrExpr;
		default:
			break;
		}
	}
	else if (CurrToken.Type == IDENTIFIER) {
		switch (NextToken.OpKwValue)
		{
		case SEP_EQUALS:
			CurrExpr = VarAssignmentParse();
			CurrExpr->Line = CurrToken.Line;
			return CurrExpr;
		case SEP_LBRACKET:
			CurrExpr = VarAssignmentArrayParse(NodeParse());
			CurrExpr->Line = CurrToken.Line;
			return CurrExpr;
		default:
			break;
		}
	}

	CurrExpr = BinExprParse();
	CurrExpr->Line = CurrToken.Line;
	return CurrExpr;
}

//========INITIALIZER========
void Parse() {
	CurrToken = TokensFirst->Tok;
	if (TokensFirst != NULL) NextToken = TokensFirst->next->Tok;

	ExprFirst = NULL;
	ExprLast = NULL;

	while (ParserEndOfTokens == false) {
		int ExprLine = CurrToken.Line;
		Expression* CurrExpr = ExpressionParse();
		CurrExpr->Line = ExprLine;

		AddExpression(CurrExpr);

		/*
		print_expression(CurrExpr);
		printf("\n");
		*/

		if (CurrToken.OpKwValue == SEP_SEMICOLON) {
			Advance();
		}
		else if (CurrExpr == NULL) {
			PrintGrammarError((GrammarError) { CurrToken.Line, CurrToken.EndColumn, "Error in expression: Null return. Check function." });
			Advance();
		}
	}

}