#include "PrintHelpers.h"

//!!!!!!!Printing methods. Only chatGpt part. Way too boring. !!!!!!!

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

void PrintVariableType(DeclarationVariableType type) {
	switch (type) {
	case VARIABLE_INT:    printf("int"); break;
	case VARIABLE_DOUBLE: printf("double"); break;
	case VARIABLE_STRING: printf("string"); break;
	case VARIABLE_BOOL:   printf("bool"); break;
	case VARIABLE_ARRAY:  printf("array"); break;
	case VARIABLE_CUSTOM: printf("custom"); break;
	case VARIABLE_AUTO:   printf("auto"); break;
	case VARIABLE_NONE:   printf("none"); break;
	default:              printf("unknown"); break;
	}
}

void print_expression(Expression* expr);

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
		if (retInfo->Value != NULL) {
			printf("Custom Type/Value: ");
			print_expression(retInfo->Value);
		}
		else
			printf("<custom?>");
		break;
	case FUNCTION_VOID:   printf("void"); break;
	default:              printf("<unknown type>"); break;
	}
}

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
		for (int i = 0; i < node->Value.FuncCall.ArgumentsNamesCount; i++) {
			PrintFunctionType(node->Value.FuncCall.Arguments[i]);
			if (i < node->Value.FuncCall.ArgumentsNamesCount - 1)
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

	PrintVariableType(d->VarType);

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