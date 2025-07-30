#include "..\Utils\Utils.h"

//GLOBAL VARIABLES
char PreviousChar = '\0';
char CurrentChar = '\0';
char NextChar = '\0';
char CurrStr[MAX_WORD_LENGHT] = "\0";

int TokenIndex = 0;
int CurrStrIndex = 0;
int ColIndex = 0;
int RowIndex = 0;

bool IsString = false;

//FUNCTION DECLARATIONS
void Lexer();

void ReadNextChar(FILE* ReadFile);

TOKEN* Lex(FILE* ReadFile);

FILE* Shell();