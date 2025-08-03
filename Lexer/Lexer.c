#define _CRT_SECURE_NO_WARNINGS
#include "Lexer.h"

//Shell to input text path file.
FILE* Shell() {
	printf("NanoHomeScript. Enter path to .nhs file.\n");
	printf("NanoHS>");

	char String[MAX_STRING_LEN];
	int PathError = Read_User_String(String, 100);
	if (PathError == -1) exit(1);

	//For debug purposes
	if (String[0] == '\0') {
		strcpy(String, "C:\\Users\\dacco\\source\\repos\\NanoHomeScript\\NanoHomeScript\\Tests\\Test.nhs");
	}


	FILE* File = fopen(String, "r");
	if(File == NULL) printf("\033[31mERROR reading file.\033[0m\n");

	return File;
}

//Reads next char, updating current, previous and next char. Also increases row and line for debug.
void ReadNextChar(FILE* ReadFile) {
	if (ReadFile == NULL) {
		GrammarError Err = { RowIndex, ColIndex, "Lex error." };
		PrintGrammarError(Err);
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

//Checks if string is a keyword (specified in Keywords)
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

//Checks if string is an INT
bool IsNumber(char* CurrStr) {
	while (*CurrStr) {
		if (*CurrStr < '0' || *CurrStr > '9')
			return false;
		++CurrStr;
	}
	return true;
}

//Returns token with specified values
TOKEN GenerateTok(TokenType TokType, TokenValue TokValue, int Line, int Col) {
	TOKEN Tok;
	Tok.Type = TokType;
	Tok.Value = TokValue;
	Tok.Line = Line;
	Tok.EndColumn = Col;

	return Tok;
}

//Adds the token to the back of the linked list. 
void AddToken(TOKEN NewTok, TokenList** TokensHead, TokenList** TokensLast) {
	TokenList* NewEntry = (TokenList*)malloc(sizeof(TokenList));
	if (NewEntry == NULL) {
		PrintGrammarError((GrammarError) { RowIndex, ColIndex, "ERROR on malloc of new Token entry." });
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

//Tries to generate a int/float token from string
TokenValue NumberHandler(char* NumCurrStr, FILE* ReadFile, TokenType* Type) {
	char numBase[MAX_WORD_LENGHT];
	strcpy(numBase, NumCurrStr);

	if (CurrentChar == '.') {
		NumCurrStr[0] = '\0';
		CurrStrIndex = 0;
		ReadNextChar(ReadFile);

		while (Separators[CurrentChar] == SEP_UNKNOWN && CurrentChar != EOF) {
			CurrStr[CurrStrIndex] = CurrentChar;
			CurrStrIndex++;
			ReadNextChar(ReadFile);
		}
		NumCurrStr[CurrStrIndex] = '\0';

		if (IsNumber(NumCurrStr) == false) {
			PrintGrammarError((GrammarError) { RowIndex, ColIndex, "ERROR, wrong number format." });
		}

		char fullnum[MAX_WORD_LENGHT + MAX_WORD_LENGHT + 2];
		fullnum[0] = '\0';

		strcat(fullnum, numBase);
		strcat(fullnum, ".");
		strcat(fullnum, NumCurrStr);
		
		double num = atof(fullnum);

		TokenValue TVal;
		TVal.doubleVal = num;
		*Type = DOUBLE;
		return TVal;
	}

	int num = atoi(NumCurrStr);

	TokenValue TVal;
	TVal.intVal = num;
	*Type = INT;
	return TVal;

}

//Appends a string token to the list, including everything between the two '"'
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

//Appends a keyword token to the list.
void GenerateKeywordToken(KeywordType KWType, TokenList** TokensHead, TokenList** TokensLast) {
	TokenValue TVal;

	TVal.OpKwValue = KWType;

	TOKEN NewTok = GenerateTok(KEYWORD, TVal, RowIndex, ColIndex);

	AddToken(NewTok, TokensHead, TokensLast);

	CurrStr[0] = '\0';
	CurrStrIndex = 0;
}

//Appends a single (OR MULTI-CHAR) operator to the list. NOTE: **,++ etc are considered 'keywords', but are lexed here.
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
		PrintGrammarError((GrammarError) { RowIndex, ColIndex, "ERROR in operator recognizment." });
	}
}

//Appends identifier token to the list.
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
			PrintGrammarError((GrammarError) { RowIndex, ColIndex, "ERROR assigning identifier string malloc." });
			return;
		}

		strcpy_s(TVal.stringVal, strlen(CurrStr) + 1, CurrStr);
		NewTok = GenerateTok(IDENTIFIER, TVal, RowIndex, ColIndex);
	}

	AddToken(NewTok, TokensHead, TokensLast);

	CurrStr[0] = '\0';
	CurrStrIndex = 0;
}

//Branches to analyze all different tokens.
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

//Lexes the complete document. The printfs are for debug only
TokenList* Lex(FILE* ReadFile) {
	TokenList* TokensHead = NULL;
	TokenList* TokensLast = NULL;

	while (CurrentChar != EOF) {
		AnalyzeTokens(&TokensHead, &TokensLast, ReadFile);
	}

	//TOKEN DEBUG INFO
	TokenList* curr = TokensHead;
	while (curr != NULL) {
		if (curr->Tok.Type == OPERATOR) printf("TOKEN: (OPERATOR, %c, line: %d, column: %d)\n", InverseSeparators[curr->Tok.Value.OpKwValue], curr->Tok.Line, curr->Tok.EndColumn);
		else if (curr->Tok.Type == KEYWORD) printf("TOKEN: (KEYWORD, %s, line: %d, column: %d)\n", Keywords[curr->Tok.Value.OpKwValue].Text, curr->Tok.Line, curr->Tok.EndColumn);
		else if (curr->Tok.Type == IDENTIFIER) printf("TOKEN: (IDENTIFIER, %s, line: %d, column: %d)\n", curr->Tok.Value.stringVal, curr->Tok.Line, curr->Tok.EndColumn);
		else if (curr->Tok.Type == STRING) printf("TOKEN: (STRING, %s, line: %d, column: %d)\n", curr->Tok.Value.stringVal, curr->Tok.Line, curr->Tok.EndColumn);
		else if (curr->Tok.Type == INT) printf("TOKEN: (INT, %d, line: %d, column: %d)\n", curr->Tok.Value.intVal, curr->Tok.Line, curr->Tok.EndColumn);
		else if (curr->Tok.Type == DOUBLE) printf("TOKEN: (DOUBLE, %lf, line: %d, column: %d)\n", curr->Tok.Value.doubleVal, curr->Tok.Line, curr->Tok.EndColumn);
		curr = curr->next;
	}

	return TokensHead;
}

//Main lexer function. Call to separate input text.
void Lexer() {
	InstantiateSepTable();
	InstantiateInverseSepTable();

	FILE *File = Shell();

	CurrentChar = fgetc(File);
	NextChar = fgetc(File);
	TokensFirst = Lex(File);
}