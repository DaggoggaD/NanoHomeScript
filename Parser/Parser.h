 #pragma once
#include "..\Utils\Utils.h"
#define IF_FUNCTION 0
#define WHILE_FUNCTION 1

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
	NODE_GROUPING,
	NODE_RETURN,
	NODE_BLOCK
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

typedef struct S_NodeBlock {
	struct Expression** Expressions;
	int Index;
	int Size;
} NodeBlock;

typedef struct S_NodeReturn {
	struct FunctionReturnInfo** ReturnNames;
	int ReturnNamesCount;
} NodeReturn;

typedef union {
	TOKEN Tok;
	NodeCall FuncCall;
	NodeIndexAccess AtArrayIndex;
	NodeBlock Block;
	NodeReturn Return;
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
	EXPRESSION_IF,
	EXPRESSION_WHILE,
	EXPRESSION_FUNC,
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
	BINARY_AND,
	BINARY_OR,
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

typedef struct S_IfExpression {
	struct Expression* Condition;
	struct Expression* IfBlock;
} IfExpression;

//Can be fusedn whith If expression. For clarity, kept separeted now.
typedef struct S_WhileExpression {
	struct Expression* Condition;
	struct Expression* WhileBlock;
} WhileExpression;


typedef enum {
	FUNCTION_INT,
	FUNCTION_DOUBLE,
	FUNCTION_STRING,
	FUNCTION_BOOL,
	FUNCTION_ARRAY,
	FUNCTION_CUSTOM,
	FUNCTION_VOID
} FunctionType;

typedef struct S_FunctionReturnInfo {
	FunctionType Type;
	char* CustomTypeName;
} FunctionReturnInfo;//Both for function return types and return itself.

typedef struct S_FunctionExpression {
	FunctionReturnInfo** ReturnTypes;
	int ReturnTypesCount;
	
	TOKEN FuncName;

	struct Expression* Arguments;
	struct Expression* FuncBlock;

} FunctionExpression;

//#####Expression main struct#####

typedef union {
	BinaryExpression* BinExpr;
	DeclarationExpression* DeclExpr;
	AssignmentExpression* AssignExpr;
	IfExpression* IfExpr;
	WhileExpression* WhileExpr;
	FunctionExpression* FuncExpr;
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