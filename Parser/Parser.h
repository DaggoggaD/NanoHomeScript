 #pragma once
#include "..\Utils\Utils.h"

TOKEN CurrToken;
TOKEN NextToken;
bool ParserEndOfTokens = false;

//==================NON TERMINALS//==================
typedef struct Expression;


//__________NODE__________
typedef enum {
	NODE_NUMBER,
	NODE_STRING,
	NODE_BOOL,
	NODE_NULL,
	NODE_IDENTIFIER,
	NODE_CALL,
	NODE_INDEX_ACCESS,
	NODE_GROUPING
} NodeType;

typedef struct S_NodeCall {
	TOKEN CallNameTok;
	struct Expression** Arguments; //Array of expressions
	int ArgsNum;
} NodeCall;

typedef struct S_NodeIndexAccess {
	TOKEN ArrayNameTok;
	struct Expression* Index;
} NodeIndexAccess;

typedef union {
	TOKEN Tok;
	NodeCall FuncCall;
	NodeIndexAccess AtArrayIndex;
	struct Expression* NodeGrouping;
} NodeValue;

typedef struct S_Node {
	NodeType Type;
	NodeValue Value;
} Node;

//__________FACTOR__________

typedef enum {
	FACTOR_NEGATIVE,
	FACTOR_NOT,
	FACTOR_NONE
} FactorType;

typedef struct S_Factor {
	FactorType Type;
	struct Expression* Value;

} Factor;

//__________TERM__________

typedef enum {
	TERM_DIVISION,
	TERM_MOLTIPLICATION,
	TERM_NONE
} TermType;

typedef struct S_Term {
	TermType Type;
	struct Expression* Left;
	struct Expression* Right;
} Term;

//__________EXPRESSIONS__________

typedef enum {
	EXPRESSION_BINARY,
	EXPRESSION_DECLARATION,
	EXPRESSION_ASSIGNMENT,
	EXPRESSION_TERM,
	EXPRESSION_FACTOR,
	EXPRESSION_NODE
} ExpressionType;

//###Sub-Expressions###
typedef enum {
	BINARY_ADD,
	BINARY_SUB,
	BINARY_LESS,
	BINARY_GREATER,
	BINARY_LOE,
	BINARY_GOE,
	BINARY_EQUAL,
	BINARY_NOE,
	BINARY_ASSIGN,
	BINARY_NONE
} BinaryExpressionType;

typedef struct S_BinExpr {
	BinaryExpressionType Type;
	struct Expression* Left;
	struct Expression* Right;
} BinaryExpression;


typedef enum {
	DECLARATION_AUTO,
	DECLARATION_WITH_TYPE
} DeclarationExpressionType;

typedef enum {
	VARIABLE_INT,
	VARIABLE_DOUBLE,
	VARIABLE_STRING,
	VARIABLE_BOOL,
	VARIABLE_ARRAY,
	VARIABLE_CUSTOM,
	VARIABLE_AUTO,
	VARIABLE_NONE
} DeclarationVariableType;

typedef struct S_DeclExpr {
	DeclarationExpressionType ExprType;
	DeclarationVariableType VarType;
	TOKEN VarName;
	struct Expression* Value;

} DeclarationExpression;

typedef struct S_AssignmentExpression {
	struct Expression* VarName;
	struct Expression* Value;
} AssignmentExpression;

//#####Expression main struct#####

typedef union {
	BinaryExpression* BinExpr;
	DeclarationExpression* DeclExpr;
	AssignmentExpression* AssignExpr;
	Term* TermExpr;
	Factor* FactorExpr;
	Node* NodeExpr;
} ExpressionValue;

typedef struct S_Expression {
	ExpressionType Type;
	ExpressionValue Value;
} Expression;


typedef struct S_ExpressionList {
	Expression Expr;
	struct S_ExpressionList* Next;
} ExpressionList;

void Parse();

Expression* BinExprParse();

Expression* ExpressionParse();