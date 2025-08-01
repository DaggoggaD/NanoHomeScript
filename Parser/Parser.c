#include "Parser.h"

//Printing method. Only chatGpt part. Way too boring.
// Helper: restituisce la stringa dell'operatore binario
const char* binary_op_to_str(BinaryExpressionType t) {
	switch (t) {
	case BINARY_ADD:      return "+";
	case BINARY_SUB:      return "-";
	case BINARY_LESS:     return "<";
	case BINARY_GREATER:  return ">";
	case BINARY_LOE:      return "<=";
	case BINARY_GOE:      return ">=";
	case BINARY_EQUAL:    return "==";
	case BINARY_NOE:      return "!=";
	case BINARY_ASSIGN:   return "=";
	default:              return "?";
	}
}

const char* term_op_to_str(TermType t) {
	switch (t) {
	case TERM_DIVISION:       return "/";
	case TERM_MOLTIPLICATION: return "*";
	default:                  return "";
	}
}

const char* factor_op_to_str(FactorType f) {
	switch (f) {
	case FACTOR_NEGATIVE: return "-";
	case FACTOR_NOT:      return "!";
	default:              return "";
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
		// Supponendo che il token contenga un campo numerico; adattalo:
		// se usi int e double separati, scegli in base a CurrToken.Type
		printf("%d", node->Value.Tok.Value.intVal); // o .intVal se intero
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
	case NODE_GROUPING:
		printf("(");
		print_expression(node->Value.NodeGrouping);
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
	default:
		printf("<unknown expr kind>");
	}
}

//END OF PRINTING METHODS


BinaryExpressionType GetBinaryExpressionType(TOKEN* Tok) {
	if (Tok==NULL) return BINARY_NONE;

	if (Tok->Type == OPERATOR) {
		switch (Tok->Value.OpKwValue)
		{
		case SEP_OP_ADD: return BINARY_ADD;
		case SEP_OP_SUB: return BINARY_SUB;
		case SEP_OP_LESS: return BINARY_LESS;
		case SEP_OP_GREAT: return BINARY_GREATER;
		case SEP_EQUALS: return BINARY_ASSIGN;
		default: break;
		}
	}
	else {
		switch (Tok->Value.OpKwValue)
		{
		case KW_COMPARE: return BINARY_EQUAL;
		case KW_COMPARE_INV: return BINARY_NOE;
		case KW_GOE: return BINARY_GOE;
		case KW_LOE: return BINARY_LOE;
		default: break;
		}
	}
	return BINARY_NONE;
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
	if (CurrNode == NULL) return NULL; //ADD ERROR PATTERNS

	CurrNode->Type = Type;
	CurrNode->Value.Tok = Tok;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) return NULL; //ADD ERROR PATTERNS (and free node)
	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}

//Could be included in maletokenNode, for clarity for now included here.
Expression* MakeGroupedNode(NodeType Type, Expression* Expr) {
	Node* CurrNode = malloc(sizeof(Node));
	if (CurrNode == NULL) return NULL; //ADD ERROR PATTERNS

	CurrNode->Type = Type;
	CurrNode->Value.NodeGrouping = Expr;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) return NULL; //ADD ERROR PATTERNS (and free node)
	CurrExpr->Type = EXPRESSION_NODE;
	CurrExpr->Value.NodeExpr = CurrNode;

	return CurrExpr;
}

Expression* MakeFactor(FactorType Type, Expression* Left){
	Factor* CurrFactor = malloc(sizeof(Factor));
	if (CurrFactor == NULL) return NULL; //ADD ERROR PATTERNS

	CurrFactor->Type = Type;
	CurrFactor->Value = Left;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) return NULL; //ADD ERROR PATTERNS
	CurrExpr->Type = EXPRESSION_FACTOR;
	CurrExpr->Value.FactorExpr = CurrFactor;

	return CurrExpr;
}

Expression* MakeTerm(TermType Type, Expression* Left, Expression* Right) {

	Term* CurrTerm = malloc(sizeof(Term));
	if (CurrTerm == NULL) return NULL;

	CurrTerm->Type = Type;
	CurrTerm->Left = Left;
	CurrTerm->Right = Right;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) return NULL; //ADD ERROR PATTERNS
	CurrExpr->Type = EXPRESSION_TERM;
	CurrExpr->Value.TermExpr = CurrTerm;

	return CurrExpr;

}

Expression* MakeBinExpr(BinaryExpressionType Type, Expression* Left, Expression* Right) {
	BinaryExpression* CurrBinExpression = malloc(sizeof(BinaryExpression));
	if (CurrBinExpression == NULL) return NULL;

	CurrBinExpression->Type = Type;
	CurrBinExpression->Left = Left;
	CurrBinExpression->Right = Right;

	Expression* CurrExpr = malloc(sizeof(Expression));
	if (CurrExpr == NULL) return NULL; //ADD ERROR PATTERNS
	CurrExpr->Type = EXPRESSION_BINARY;
	CurrExpr->Value.BinExpr = CurrBinExpression;

	return CurrExpr;
}


Expression* NodeParse() {
	//CONVERT TO SWITCH

	if (CurrToken.Type == DOUBLE || CurrToken.Type == INT) {
		Expression* NumNode = MakeTokenNode(NODE_NUMBER, CurrToken);
		Advance();
		return NumNode;
	}
	else if (CurrToken.Value.OpKwValue == SEP_LPAREN) {
		Advance();
		Expression* Grouped = BinExprParse(); //CHANGE THIS TO PARSE, WHEN FUNCTION IS COMPLETE

		if (CurrToken.Value.OpKwValue != SEP_RPAREN) return NULL; //ADD ERROR PATTERNS

		Advance();
		return MakeGroupedNode(NODE_GROUPING, Grouped);
	}
	return NULL;
}

Expression* FactorParse() {
	if (CurrToken.Value.OpKwValue == SEP_OP_NOT) {
		Advance();
		Expression* Left = FactorParse();
		return MakeFactor(FACTOR_NOT, Left);
	}
	else if (CurrToken.Value.OpKwValue == SEP_OP_SUB) {
		Advance();
		Expression* Left = FactorParse();
		return MakeFactor(FACTOR_NEGATIVE, Left);
	}
	
	return NodeParse();
}

Expression* TermParse() {
	Expression* Left = FactorParse();

	while (CurrToken.Value.OpKwValue == SEP_OP_MULT || CurrToken.Value.OpKwValue == SEP_OP_DIV) {
		TermType Type;
		if (CurrToken.Value.OpKwValue == SEP_OP_MULT) Type = TERM_MOLTIPLICATION;
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

void Parse() {
	CurrToken = TokensFirst->Tok;
	if (TokensFirst != NULL) NextToken = TokensFirst->next->Tok;

	while (ParserEndOfTokens==false) {
		Expression* CurrExpr = BinExprParse();
		print_expression(CurrExpr);
		printf("\n");

		if (CurrToken.Value.OpKwValue == SEP_SEMICOLON) {
			Advance();
		}
		else if(CurrExpr==NULL) {
			Advance();
		}
	}
	
}