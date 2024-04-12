#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];
static int isVariableName(char c){
    return isdigit(c) || c=='_'||isalpha(c);
}
TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        if(i>=MAXLEN){//TODO: check is there an error type is for this case
            fprintf(stderr, "Number too long\n");
            exit(0);
        }
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        char next_c = fgetc(stdin);
        lexeme[0] = c;
        if(next_c==c){
            lexeme[1] = next_c;
            lexeme[2] = '\0';
            return INCDEC;
        }else if(next_c=='='){
            lexeme[1] = next_c;
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        }else{
            lexeme[1] = '\0';
            ungetc(next_c, stdin);//push back next_c
            return ADDSUB;
        }
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (c == EOF) {
        return ENDFILE;
    } else if(c=='&'){
        strcpy(lexeme, "&");
        return AND;
    } else if(c=='|'){
        strcpy(lexeme, "|");
        return OR;    
    } else if(c=='^'){
        strcpy(lexeme, "^");
        return XOR;
    } else if(isVariableName(c)){
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isVariableName(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        if(i>=MAXLEN){
            fprintf(stderr, "Variable name too long\n");
            exit(0);
        }
        return ID; 
    }else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}
