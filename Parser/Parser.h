#pragma once
#include "..\Utils\Utils.h"
#define IF_FUNCTION 0
#define WHILE_FUNCTION 1


//ENUMERATORS
typedef enum {
	NODE_NUMBER, //done
	NODE_STRING, //done
	NODE_BOOL,
	NODE_NULL, 
	NODE_IDENTIFIER, //done
	NODE_CALL,	//mostly done (missing return)
	NODE_INDEX_ACCESS,
	NODE_GROUPING, //done
	NODE_RETURN,
	NODE_BLOCK	//done
} NodeType;

typedef enum {
	FACTOR_NEGATIVE,
	FACTOR_NOT,
	FACTOR_NONE
} FactorType;

typedef enum {
	TERM_DIVISION,
	TERM_MOLTIPLICATION,
	TERM_NONE
} TermType;

typedef enum {
	EXPRESSION_NODE,
	EXPRESSION_TERM,
	EXPRESSION_FACTOR,
	EXPRESSION_BINARY,
	EXPRESSION_DECLARATION,
	EXPRESSION_ASSIGNMENT,
	EXPRESSION_IF,
	EXPRESSION_WHILE,
	EXPRESSION_FUNC
} ExpressionType;

typedef enum {
	BINARY_ADD,
	BINARY_SUB,
	BINARY_LESS,
	BINARY_GREATER,
	BINARY_LOE,
	BINARY_GOE,
	BINARY_EQUAL,
	BINARY_NOE,
	BINARY_AND,
	BINARY_OR,
	BINARY_MOD,
	BINARY_NONE
} BinaryExpressionType;

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

typedef enum {
	FUNCTION_INT,
	FUNCTION_DOUBLE,
	FUNCTION_STRING,
	FUNCTION_BOOL,
	FUNCTION_ARRAY,
	FUNCTION_CUSTOM,
	FUNCTION_VOID
} FunctionType;


//Structures and Unions

//Nodes
typedef struct S_Expression Expression;


typedef struct S_NodeCall {
	TOKEN CallNameTok;
	struct FunctionReturnInfo** Arguments; //Array of expressions
	int ArgumentsNamesCount;
} NodeCall;

typedef struct S_NodeIndexAccess {
	TOKEN ArrayNameTok;
	struct Expression* Index;
} NodeIndexAccess;

typedef struct S_NodeBlock {
	struct S_Expression** Expressions;
	int Index;
	int Size;
} NodeBlock;

typedef struct S_NodeReturn {
	struct S_FunctionReturnInfo** ReturnNames;
	int ReturnNamesCount;
} NodeReturn;

typedef union {
	TOKEN Tok;
	NodeCall FuncCall;
	NodeIndexAccess AtArrayIndex;
	NodeBlock Block;
	NodeReturn Return;
	struct S_Expression* NodeGrouping;
} NodeValue;

typedef struct S_Node {
	NodeType Type;
	NodeValue Value;
} Node;

//Factor
typedef struct S_Factor {
	FactorType Type;
	struct Expression* Value;
} Factor;

//Term
typedef struct S_Term {
	TermType Type;
	struct Expression* Left;
	struct Expression* Right;
} Term;

//Expressions
typedef struct S_BinExpr {
	BinaryExpressionType Type;
	struct Expression* Left;
	struct Expression* Right;
} BinaryExpression;

typedef struct S_DeclExpr {
	DeclarationVariableType VarType;
	TOKEN VarName;
	struct Expression* Value;

} DeclarationExpression;

typedef struct S_AssignmentExpression {
	struct S_Expression* VarName;
	struct S_Expression* Value;
} AssignmentExpression;

typedef struct S_IfExpression {
	struct Expression* Condition;
	struct Expression* IfBlock;
} IfExpression;

typedef struct S_WhileExpression {
	//Can be fusedn whith If expression. For clarity, kept separeted now.
	struct Expression* Condition;
	struct Expression* WhileBlock;
} WhileExpression;

typedef struct S_FunctionReturnInfo {
	//Both for function return types, return itself and function calls.
	FunctionType Type;
	struct S_Expression* Value;
} FunctionReturnInfo;

typedef struct S_FunctionExpression {
	TOKEN FuncName;
	
	FunctionReturnInfo** ReturnTypes;
	int ReturnTypesCount;

	struct Expression* Arguments;
	struct Expression* FuncBlock;

} FunctionExpression;

//Expression parent struct
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
	int Line;
} Expression;

typedef struct S_ExpressionList {
	Expression* Expr;
	struct S_ExpressionList* Next;
} ExpressionList;

//Global Variables

ExpressionList* ExprFirst;
ExpressionList* ExprLast;

//Functions
void Parse();

Expression* BinExprParse();

Expression* ExpressionParse();

Expression* NodeParse();