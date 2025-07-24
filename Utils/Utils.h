#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

typedef enum {
	IDENTIFIER,
	OPERATOR,
	KEYWORD,
	INT,
	DOUBLE,
	STRING,
	CHAR
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
	int Line;
	int BeginColumn;
} TOKEN;

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