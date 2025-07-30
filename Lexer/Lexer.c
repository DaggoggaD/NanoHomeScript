#define _CRT_SECURE_NO_WARNINGS
#include "Lexer.h"

FILE* Shell() {
	printf("NanoHomeScript. Enter path to .nhs file.\n");
	printf("NanoHS>");

	char String[MAX_STRING_LEN];
	int PathError = Read_User_String(String, 100);
	if (PathError == -1) exit(1);

	//For debug purposes
	if (String[0] == '\0') {
		strcpy(String, "PRIVATE PATH");
	}


	FILE* File = fopen(String, "r");
	if(File == NULL) printf("\033[31mERROR reading file.\033[0m\n");

	return File;
}

void ReadNextChar(FILE* ReadFile) {
	if (ReadFile == NULL) {
		LexError Err = { RowIndex, ColIndex, "Lex error." };
		PrintLexError(Err);
		return;
	}

	if (CurrentChar != EOF) {
		PreviousChar = CurrentChar;
		CurrentChar = NextChar;
		NextChar = fgetc(ReadFile);
	}

	if (CurrentChar == '\n') {
		RowIndex++;
		ColIndex = 0;
		ReadNextChar(ReadFile);
	}
	else if (CurrentChar == '\t') {
		ColIndex++;
		ReadNextChar(ReadFile);
	}
	else ColIndex++;
}

bool IsKeyword(char* CurrStr, KeywordType *Type) {
	for (int i = 0; i < KW_UNKNOWN; i++)
	{
		if (strcmp(Keywords[i].Text, CurrStr)==0) {
			*Type = Keywords[i].Type;
			return true;
		}
	}
	return false;
}

bool IsNumber(char* CurrStr) {
	while (*CurrStr) {
		if (*CurrStr < '0' || *CurrStr > '9')
			return false;
		++CurrStr;
	}
	return true;
}

TOKEN GenerateTok(TokenType TokType, TokenValue TokValue, int Line, int Col) {
	TOKEN Tok;
	Tok.Type = TokType;
	Tok.Value = TokValue;
	Tok.Line = Line;
	Tok.EndColumn = Col;

	return Tok;
}

void AddToken(TOKEN NewTok, TokenList** TokensHead, TokenList** TokensLast) {
	TokenList* NewEntry = (TokenList*)malloc(sizeof(TokenList));
	if (NewEntry == NULL) {
		PrintLexError((LexError) { RowIndex, ColIndex, "ERROR on malloc of new Token entry." });
		return;
	}

	NewEntry->Tok = NewTok;
	NewEntry->next = NULL;

	if (*TokensHead == NULL) {
		*TokensHead = NewEntry;
		*TokensLast = NewEntry;
	}
	else {
		(*TokensLast)->next = NewEntry;
		*TokensLast = NewEntry;
	}
}

TokenValue NumberHandler(char* CurrStr, FILE* ReadFile, TokenType* Type) {
	char numBase[MAX_WORD_LENGHT];
	strcpy(numBase, CurrStr);

	if (CurrentChar == '.') {
		CurrStr[0] = '\0';
		CurrStrIndex = 0;
		ReadNextChar(ReadFile);

		while (Separators[CurrentChar] == SEP_UNKNOWN && CurrentChar != EOF) {
			CurrStr[CurrStrIndex] = CurrentChar;
			CurrStrIndex++;
			ReadNextChar(ReadFile);
		}
		CurrStr[CurrStrIndex] = '\0';

		if (IsNumber(CurrStr) == false) {
			PrintLexError((LexError) { RowIndex, ColIndex, "ERROR, wrong number format." });
		}

		char fullnum[MAX_WORD_LENGHT + MAX_WORD_LENGHT + 2];
		fullnum[0] = '\0';

		strcat(fullnum, numBase);
		strcat(fullnum, ".");
		strcat(fullnum, CurrStr);
		
		double num = atof(fullnum);

		TokenValue TVal;
		TVal.doubleVal = num;
		*Type = DOUBLE;
		return TVal;
	}

	int num = atoi(CurrStr);

	TokenValue TVal;
	TVal.intVal = num;
	*Type = INT;
	return TVal;

}

void GenerateStringToken(TokenList** TokensHead, TokenList** TokensLast, FILE* ReadFile) {
	IsString = true;
	CurrStr[0] = "\0";
	CurrStrIndex = 0;
	ReadNextChar(ReadFile);

	while (Separators[CurrentChar] != SEP_DUB_QUOTE && CurrentChar != EOF) {
		CurrStr[CurrStrIndex] = CurrentChar;
		CurrStrIndex++;
		ReadNextChar(ReadFile);
	}

	IsString = false;
	CurrStr[CurrStrIndex] = '\0';

	TokenValue TVal;
	TVal.stringVal = malloc(sizeof(char) * CurrStrIndex);

	strcpy(TVal.stringVal, CurrStr);

	TOKEN NewTok = GenerateTok(STRING, TVal, RowIndex, ColIndex);

	AddToken(NewTok, TokensHead, TokensLast);
	CurrStr[0] = '\0';
	CurrStrIndex = 0;
}

void GenerateKeywordToken(KeywordType KWType, TokenList** TokensHead, TokenList** TokensLast) {
	TokenValue TVal;

	TVal.OpKwValue = KWType;

	TOKEN NewTok = GenerateTok(KEYWORD, TVal, RowIndex, ColIndex);

	AddToken(NewTok, TokensHead, TokensLast);

	CurrStr[0] = '\0';
	CurrStrIndex = 0;
}

void GeneratorSMOperator(TokenList** TokensHead, TokenList** TokensLast, FILE* ReadFile) {
	KeywordType KWType;
	char CheckMultiString[] = { CurrentChar, NextChar, '\0' };

	if (IsKeyword(CheckMultiString, &KWType))
	{
		TokenValue TVal;

		TVal.OpKwValue = KWType;

		TOKEN NewTok = GenerateTok(KEYWORD, TVal, RowIndex, ColIndex);

		AddToken(NewTok, TokensHead, TokensLast);
		ReadNextChar(ReadFile);
		ReadNextChar(ReadFile);
	}
	else if (CurrentChar != ' ' && Separators[CurrentChar] != SEP_UNKNOWN) {

		if (CurrentChar == '"') {
			GenerateStringToken(TokensHead, TokensLast, ReadFile);
		}
		else {
			TokenValue TVal;
			TVal.OpKwValue = Separators[CurrentChar];

			TOKEN NewTok = GenerateTok(OPERATOR, TVal, RowIndex, ColIndex);

			AddToken(NewTok, TokensHead, TokensLast);
		}
		
		ReadNextChar(ReadFile);
	}
	else if (CurrentChar == ' ') {
		ReadNextChar(ReadFile);
	}
	else {
		PrintLexError((LexError) { RowIndex, ColIndex, "ERROR in operator recognizment." });
	}
}

void GenerateIdentifierToken(TokenList** TokensHead, TokenList** TokensLast, FILE* ReadFile) {
	TOKEN NewTok;
	KeywordType KWType;

	if (IsNumber(CurrStr)) {
		TokenValue TVal = NumberHandler(CurrStr, ReadFile, &KWType);
		if (KWType == INT) NewTok = GenerateTok(INT, TVal, RowIndex, ColIndex);
		else NewTok = GenerateTok(DOUBLE, TVal, RowIndex, ColIndex);
	}
	else {
		TokenValue TVal;
		TVal.stringVal = malloc(strlen(CurrStr) + 1);
		if (TVal.stringVal == NULL) {
			PrintLexError((LexError) { RowIndex, ColIndex, "ERROR assigning identifier string malloc." });
			return;
		}

		strcpy_s(TVal.stringVal, strlen(CurrStr) + 1, CurrStr);
		NewTok = GenerateTok(IDENTIFIER, TVal, RowIndex, ColIndex);
	}

	AddToken(NewTok, TokensHead, TokensLast);

	CurrStr[0] = '\0';
	CurrStrIndex = 0;
}

void AnalyzeTokens(TokenList** TokensHead, TokenList** TokensLast, FILE* ReadFile) {
	KeywordType KWType;

	while (Separators[CurrentChar] == SEP_UNKNOWN && CurrentChar != EOF ) {
		CurrStr[CurrStrIndex] = CurrentChar;
		CurrStrIndex++;
		ReadNextChar(ReadFile);
	}
	CurrStr[CurrStrIndex] = '\0';

	if (CurrentChar == EOF) return;


	if (CurrStrIndex == 0) {
		GeneratorSMOperator(TokensHead, TokensLast, ReadFile);
	}
	else if (IsKeyword(CurrStr, &KWType)) {
		GenerateKeywordToken(KWType, TokensHead, TokensLast);
	}
	else {
		GenerateIdentifierToken(TokensHead, TokensLast, ReadFile);
	}
	return;
}

TOKEN* Lex(FILE* ReadFile) {
	TokenList* TokensHead = NULL;
	TokenList* TokensLast = NULL;

	while (CurrentChar != EOF) {
		AnalyzeTokens(&TokensHead, &TokensLast, ReadFile);
	}

	//TOKEN DEBUG INFO
	while (TokensHead != NULL) {
		if (TokensHead->Tok.Type == OPERATOR) printf("TOKEN: (OPERATOR, %c, line: %d, column: %d)\n", InverseSeparators[TokensHead->Tok.Value.OpKwValue], TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == KEYWORD) printf("TOKEN: (KEYWORD, %s, line: %d, column: %d)\n", Keywords[TokensHead->Tok.Value.OpKwValue].Text, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == IDENTIFIER) printf("TOKEN: (IDENTIFIER, %s, line: %d, column: %d)\n", TokensHead->Tok.Value.stringVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == STRING) printf("TOKEN: (STRING, %s, line: %d, column: %d)\n", TokensHead->Tok.Value.stringVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == INT) printf("TOKEN: (INT, %d, line: %d, column: %d)\n", TokensHead->Tok.Value.intVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == DOUBLE) printf("TOKEN: (DOUBLE, %lf, line: %d, column: %d)\n", TokensHead->Tok.Value.doubleVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		TokensHead = TokensHead->next;
	}
}

//Main lexer function. Call to separate input text.
void Lexer() {
	InstantiateSepTable();
	InstantiateInverseSepTable();

	FILE *File = Shell();

	CurrentChar = fgetc(File);
	NextChar = fgetc(File);
	TOKEN* Tokens = Lex(File);
}

int main() {
	Lexer();
}