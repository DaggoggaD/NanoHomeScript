#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STRING_LEN 100
#define MAX_CHARS 256
#define MAX_WORD_LENGHT 50

//==================ERRORS==================
typedef struct {
    int Line;
    int Column;

    char* ErrorText;
} GrammarError;

//==================TOKEN TYPES, VALUES AND STRUCTS==================

typedef enum {
    IDENTIFIER = 0,
    OPERATOR = 1,
    KEYWORD = 2,
    INT = 3,
    DOUBLE = 4,
    STRING = 5,
    CHAR = 6,
    UNKNOWN = 7
} TokenType;

typedef union {
	int intVal;
	double doubleVal;
	char charVal;
	char* stringVal;
} TokenValue;

typedef struct {
	TokenType Type;
	TokenValue Value;
    int OpKwValue;
	int Line;
	int EndColumn;
} TOKEN;

typedef struct TokenList{
    TOKEN Tok;
    struct TokenList* next;
} TokenList;


//==================ONE CHAR SEPARATORS==================
typedef enum {
    SEP_COMMA,     // ,
    SEP_SEMICOLON, // ;
    SEP_LPAREN,    // (
    SEP_RPAREN,    // )
    SEP_LBRACE,    // {
    SEP_RBRACE,    // }
    SEP_LBRACKET,  // [
    SEP_RBRACKET,  // ]
    SEP_EQUALS,    // =
    SEP_COLON,     // :
    SEP_DOT,       // .
    SEP_SPACE,     // ' '
    SEP_QUOTE,     // '
    SEP_DUB_QUOTE, // "
    SEP_OP_MULT,   // *
    SEP_OP_DIV,    // /
    SEP_OP_ADD,    // +
    SEP_OP_SUB,    // -
    SEP_OP_LESS,   // <
    SEP_OP_GREAT,  // >
    SEP_OP_NOT,    // !

    SEP_UNKNOWN    //Nan
} SeparatorType;


//==================KEYWORDS==================
typedef enum {
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,
    KW_VAR,

    KW_FUNC,
    KW_AND,
    KW_OR,
    KW_FOREACH,
    KW_IN,
    KW_IMPORT,
    KW_DEFINE,
    KW_CONSTANT,

    KW_INT,
    KW_DOUBLE,
    KW_STRING,
    KW_CHAR,
    KW_VOID,
    KW_ARRAY,

    KW_COMMENT,
    KW_COMPARE,
    KW_COMPARE_INV,
    KW_POW,
    KW_INCREASE,
    KW_DECREASE,
    KW_GOE,
    KW_LOE,

    KW_UNKNOWN
} KeywordType;

typedef struct {
    const char* Text;
    KeywordType Type;
} KeywordEntry;


//==================VARIABLES==================
extern SeparatorType Separators[MAX_CHARS];
extern char InverseSeparators[MAX_CHARS];

extern KeywordEntry Keywords[];
TokenList* TokensFirst;

//==================FUNCTIONS==================
int Read_User_String(char* StringBuffer, int MaxLen);

void InstantiateSepTable();

void PrintGrammarError(GrammarError Error);
void PrintGrammarWarning(GrammarError Error);
bool CompareOperator(TOKEN Tok, int compare);

void PrintToken(TOKEN Tok);