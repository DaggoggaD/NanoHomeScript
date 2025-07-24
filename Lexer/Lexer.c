#include "Lexer.h"

FILE* Shell() {
	printf("NanoHomeScript. Enter path to .nhs file.\n");
	printf("NanoHS>");

	char String[MAX_STRING_LEN];
	int PathError = Read_User_String(String, 100);
	if (PathError == -1) exit(1);

	FILE* File = fopen(String, "r");
	if(File == NULL) printf("\033[31mERROR reading file.\033[0m\n");

}

void Lexer() {
	FILE *File = Shell();
}

int main() {
	Lexer();
}