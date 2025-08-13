#include "Utils.h"

//Used to check if string is a separator. Inverse is used to get the index from a certain char type
SeparatorType Separators[MAX_CHARS];
char InverseSeparators[MAX_CHARS];

//Keywords dict
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
    {"void", KW_VOID},
    {"array", KW_ARRAY}, //Can be splitted in arrayInt, arrayFloat... Not suggested -> array[struct] wont be possible. stick with dinamic.

    {"//", KW_COMMENT},
    {"==", KW_COMPARE},
    {"!=", KW_COMPARE_INV},
    {"**", KW_POW},
    {">=", KW_GOE},
    {"<=", KW_LOE},
    {"++", KW_INCREASE},
    {"--", KW_DECREASE},

    {NULL, KW_UNKNOWN} // fine della lista
};

//Reads the user string. Returns 0 if functions completes correctly, -1 otherwise.ù
int Read_User_String(char* StringBuffer, int MaxLen) {
	if (fgets(StringBuffer, MaxLen, stdin)) {
		size_t StringLenght = strlen(StringBuffer);
		if (StringLenght > 0 && StringBuffer[StringLenght - 1] == '\n') {
			StringBuffer[StringLenght - 1] = '\0';
            return 0;
		}
		else {
			StringBuffer[0] = '\0';
			printf("\033[33mWARNING! String empty or too big.\033[0m\n");
            return -1;
		}
	}
	else {
		printf("\033[31mERROR reading user string.\033[0m\n");
		return -1;
	}
}

//Used for fast acces to separator indexes
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
    Separators['!'] = SEP_OP_NOT;
}

//Inverse action
void InstantiateInverseSepTable() {
    for (int i = 0; i < MAX_CHARS; i++)
        InverseSeparators[i] = '_';  // default ignoto

    InverseSeparators[SEP_COMMA] = ',';
    InverseSeparators[SEP_SEMICOLON] = ';';
    InverseSeparators[SEP_LPAREN] = '(';
    InverseSeparators[SEP_RPAREN] = ')';
    InverseSeparators[SEP_LBRACE] = '{';
    InverseSeparators[SEP_RBRACE] = '}';
    InverseSeparators[SEP_LBRACKET] = '[';
    InverseSeparators[SEP_RBRACKET] = ']';
    InverseSeparators[SEP_EQUALS] = '=';
    InverseSeparators[SEP_COLON] = ':';
    InverseSeparators[SEP_DOT] = '.';
    InverseSeparators[SEP_SPACE] = ' ';
    InverseSeparators[SEP_QUOTE] = '\'';
    InverseSeparators[SEP_DUB_QUOTE] = '"';
    InverseSeparators[SEP_OP_MULT] = '*';
    InverseSeparators[SEP_OP_DIV] = '/';
    InverseSeparators[SEP_OP_ADD] = '+';
    InverseSeparators[SEP_OP_SUB] = '-';
    InverseSeparators[SEP_OP_LESS] = '<';
    InverseSeparators[SEP_OP_GREAT] = '>';
}

//Simply prints the error
void PrintGrammarError(GrammarError Error) {
    printf("\033[31mError at (R, L): %d,%d, INFO: %s\033[0m\n", Error.Line, Error.Column, Error.ErrorText);
    exit(1);
}

void PrintGrammarWarning(GrammarError Error) {
    printf("\033[33mError at (R, L): %d,%d, INFO: %s\033[0m\n", Error.Line, Error.Column, Error.ErrorText);
}

void PrintToken(TOKEN Tok) {
    if (Tok.Type == OPERATOR) printf("TOKEN: (OPERATOR, %c, line: %d, column: %d)\n", InverseSeparators[Tok.OpKwValue], Tok.Line, Tok.EndColumn);
    else if (Tok.Type == KEYWORD) printf("TOKEN: (KEYWORD, %s, line: %d, column: %d)\n", Keywords[Tok.OpKwValue].Text, Tok.Line, Tok.EndColumn);
    else if (Tok.Type == IDENTIFIER) printf("TOKEN: (IDENTIFIER, %s, line: %d, column: %d)\n", Tok.Value.stringVal, Tok.Line, Tok.EndColumn);
    else if (Tok.Type == STRING) printf("TOKEN: (STRING, %s, line: %d, column: %d)\n", Tok.Value.stringVal, Tok.Line, Tok.EndColumn);
    else if (Tok.Type == INT) printf("TOKEN: (INT, %d, line: %d, column: %d)\n", Tok.Value.intVal, Tok.Line, Tok.EndColumn);
    else if (Tok.Type == DOUBLE) printf("TOKEN: (DOUBLE, %lf, line: %d, column: %d)\n", Tok.Value.doubleVal, Tok.Line, Tok.EndColumn);
}

bool CompareOperator(TOKEN Tok, int compare) {
    if (Tok.Type == OPERATOR && Tok.OpKwValue == SEP_RBRACE) return true;
    return false;
}

int main() {
    Lexer();
    Parse();
    Execute();
    printf("\033[0;32m\nProgram executed successfully!\033[0m");
    char c = _getch();
    return 0;
}