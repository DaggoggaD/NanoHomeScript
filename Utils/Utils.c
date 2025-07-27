#include "Utils.h"

SeparatorType Separators[MAX_CHARS];

KeywordEntry Keywords[] = {
    {"if", KW_IF},
    {"else", KW_ELSE},
    {"while", KW_WHILE},
    {"return", KW_RETURN},
    {"var", KW_VAR},

    {"func", KW_FUNC},
    {"and", KW_AND},
    {"or", KW_OR},
    {"foreach", KW_FOREACH},
    {"in", KW_IN},
    {"import", KW_IMPORT},
    {"define", KW_DEFINE},
    {"constant", KW_CONSTANT},

    {"int", KW_INT},
    {"double", KW_DOUBLE},
    {"string", KW_STRING},
    {"char", KW_CHAR},

    {"==", KW_COMPARE},
    {"!=", KW_COMPARE_INV},
    {"**", KW_POW},
    {"++", KW_INCREASE},
    {"--", KW_DECREASE},

    {NULL, KW_UNKNOWN} // fine della lista
};

int Read_User_String(char* StringBuffer, int MaxLen) {
	if (fgets(StringBuffer, MaxLen, stdin)) {
		size_t StringLenght = strlen(StringBuffer);
		if (StringLenght > 0 && StringBuffer[StringLenght - 1] == '\n') {
			StringBuffer[StringLenght - 1] = '\0';
		}
		else {
			StringBuffer[0] = '\0';
			printf("\033[33mWARNING! String empty or too big.\033[0m\n");
		}
		return 0;
	}
	else {
		printf("\033[31mERROR reading user string.\033[0m\n");
		return -1;
	}
}

void InstantiateSepTable() {
    for (int i = 0; i < MAX_CHARS; i++)
    {
        Separators[i] = SEP_UNKNOWN;
    }
    Separators[','] = SEP_COMMA;
    Separators[';'] = SEP_SEMICOLON;
    Separators['('] = SEP_LPAREN;
    Separators[')'] = SEP_RPAREN;
    Separators['{'] = SEP_LBRACE;
    Separators['}'] = SEP_RBRACE;
    Separators['['] = SEP_LBRACKET;
    Separators[']'] = SEP_RBRACKET;
    Separators['='] = SEP_EQUALS;
    Separators[':'] = SEP_COLON;
    Separators['.'] = SEP_DOT;
    Separators[' '] = SEP_SPACE;
    Separators['\''] = SEP_QUOTE;
    Separators['"'] = SEP_DUB_QUOTE;
    Separators['*'] = SEP_OP_MULT;
    Separators['/'] = SEP_OP_DIV;
    Separators['+'] = SEP_OP_ADD;
    Separators['-'] = SEP_OP_SUB;
    Separators['<'] = SEP_OP_LESS;
    Separators['>'] = SEP_OP_GREAT;
}

void PrintLexError(LexError Error) {
    printf("\033[31mError at (R, L): %d,%d, INFO: %s\033[0m\n", Error.Line, Error.Column, Error.ErrorText);
}