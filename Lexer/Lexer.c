#define _CRT_SECURE_NO_WARNINGS
#include "Lexer.h"

FILE* Shell() {
	printf("NanoHomeScript. Enter path to .nhs file.\n");
	printf("NanoHS>");

	char String[MAX_STRING_LEN];
	int PathError = Read_User_String(String, 100);
	if (PathError == -1) exit(1);

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
		if (CurrentChar != ' ' && Separators[CurrentChar] != SEP_UNKNOWN) {
			TokenValue TVal;
			TVal.operatorVal = CurrentChar;

			TOKEN NewTok = GenerateTok(OPERATOR, TVal, RowIndex, ColIndex);

			AddToken(NewTok, TokensHead, TokensLast);
			ReadNextChar(ReadFile);
		}
		else if (CurrentChar == ' ') {
			ReadNextChar(ReadFile);
		}
		return;
	}
	else if (IsKeyword(CurrStr, &KWType)) {
		TokenValue TVal;

		TVal.keywordVal = malloc(strlen(CurrStr) + 1);
		if(TVal.keywordVal == NULL) {
			PrintLexError((LexError) { RowIndex, ColIndex, "ERROR assigning keyword string malloc." });
			return;
		}

		strcpy_s(TVal.keywordVal, strlen(CurrStr)+1, CurrStr);
		TOKEN NewTok = GenerateTok(KEYWORD, TVal, RowIndex, ColIndex);

		AddToken(NewTok, TokensHead, TokensLast);

		CurrStr[0] = '\0';
		CurrStrIndex = 0;
		return;
	}

	else {
		TokenValue TVal;

		TVal.stringVal = malloc(strlen(CurrStr) + 1);
		if (TVal.stringVal == NULL) {
			PrintLexError((LexError) { RowIndex, ColIndex, "ERROR assigning identifier string malloc." });
			return;
		}

		strcpy_s(TVal.stringVal, strlen(CurrStr) + 1, CurrStr);
		TOKEN NewTok = GenerateTok(IDENTIFIER, TVal, RowIndex, ColIndex);

		AddToken(NewTok, TokensHead, TokensLast);

		CurrStr[0] = '\0';
		CurrStrIndex = 0;
		return;
	}
}

TOKEN* Lex(FILE* ReadFile) {
	
	TokenList* TokensHead = NULL;
	TokenList* TokensLast = NULL;

	while (CurrentChar != EOF) {
		AnalyzeTokens(&TokensHead, &TokensLast, ReadFile);
	}

	//TOKEN DEBUG INFO
	while (TokensHead != NULL) {
		if (TokensHead->Tok.Type == OPERATOR) printf("TOKEN: (OPERATOR, %c, %d, %d)\n", TokensHead->Tok.Value.operatorVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == KEYWORD) printf("TOKEN: (KEYWORD, %s, %d, %d)\n", TokensHead->Tok.Value.keywordVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		else if (TokensHead->Tok.Type == IDENTIFIER) printf("TOKEN: (IDENTIFIER, %s, %d, %d)\n", TokensHead->Tok.Value.stringVal, TokensHead->Tok.Line, TokensHead->Tok.EndColumn);
		TokensHead = TokensHead->next;
	}
}

//Main lexer function. Call to separate input text.
void Lexer() {
	InstantiateSepTable();

	FILE *File = Shell();

	CurrentChar = fgetc(File);
	NextChar = fgetc(File);
	TOKEN* Tokens = Lex(File);

}

int main() {
	Lexer();
}