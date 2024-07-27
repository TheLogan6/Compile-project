#pragma once
#include "global.h"
#include "exception.h"
#include <iostream>
#include <string>

using namespace std;

struct Sem
{
    string str;
    LexType tok;
};

//Token
struct Token {
    int line;
    Sem wd;   //token语义
    int index = -1;
};

//保留字初始化
extern Sem reservedWords[21];

typedef LexType Terminal;

enum StateType { s0 = 0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13 };

extern void initReservedWord();

extern string enumToString(int lexNum);

extern bool isReserved(string ch);
extern bool isLetter(char ch);
extern bool isDigit(char ch);
extern bool isOperator(char ch);
extern Sem getReserved(string s);
extern bool isBlank(char ch);
extern int firstCharacter(char ch);

extern void lexicalAnalyse(FILE* fp);

extern void printTokenList();

extern void readToken();

extern bool ifLexicalPass;
