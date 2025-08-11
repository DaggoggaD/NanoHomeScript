#pragma once
#include "../Parser/Parser.h"

const char* binary_op_to_str(BinaryExpressionType t);

const char* term_op_to_str(TermType t);

const char* factor_op_to_str(FactorType f);

static const char* decl_var_type_to_str(DeclarationVariableType t);

void PrintFunctionType(FunctionReturnInfo* retInfo);

void print_node(Node* node);

void print_factor(Factor* factor);

void print_term(Term* term);

void print_binary_expr(BinaryExpression* bin);

void print_decl_expr(DeclarationExpression* d);

void print_assignment_expr(AssignmentExpression* a);

void print_if_expression(IfExpression* a);

void print_while_expression(WhileExpression* a);

void print_function_expression(FunctionExpression* funcExpr);

void print_expression(Expression* expr);