#include "global.h"
#include "lexical.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>



#define LEXOPENFILEDIR "../Reference/lexicalinputtest.txt"
#define LEXOUTPUTFILEDIR "../lexicaloutput.txt"

using namespace std;

Sem reservedWords[21];
extern Token tokenList[1010];

bool ifLexicalPass = true;
//#define LEXICALMAIN
#ifdef LEXICALMAIN
int main(){
    initReservedWord(); //初始化保留字
    FILE *fp;
    std::ofstream outputFile(LEXOUTPUTFILEDIR, std::ios::out | std::ios::binary);
    if(outputFile.is_open()){
        outputFile << u8"\uFEFF"; // 写入 BOM (Byte Order Mark)，表示文件以 UTF-8 编码
    }

    freopen(LEXOUTPUTFILEDIR, "w", stdout); // 输出重定向
    fp = fopen(LEXOPENFILEDIR, "r");
    fflush(stdout);
    lexicalAnalyse(fp);
    printTokenList();
    fclose(fp);

//    if(ifLexicalPass == 0)
//        longjmp(jump_buffer, 1);
}
#endif

//enum变量反解析
string enumToString(int lexvalue){
    switch (lexvalue) {
        case ENDFILE: return "ENDFILE";
        case ERROR: return "ERROR";
        case PROGRAM: return "PROGRAM";
        case PROCEDURE: return "PROCEDURE";
        case TYPE: return "TYPE";
        case VAR: return "VAR";
        case IF: return "IF";
        case THEN: return "THEN";
        case ELSE: return "ELSE";
        case FI: return "FI";
        case WHILE: return "WHILE";
        case DO: return "DO";
        case ENDWH: return "ENDWH";
        case BEGIN: return "BEGIN";
        case END1: return "END1";
        case READ: return "READ";
        case WRITE: return "WRITE";
        case ARRAY: return "ARRAY";
        case OF: return "OF";
        case RECORD: return "RECORD";
        case RETURN1: return "RETURN1";
        case INTEGER: return "INTEGER";
        case CHAR: return "CHAR";
        case ID: return "ID";
        case INTC: return "INTC";
        case CHARC: return "CHARC";
        case ASSIGN: return "ASSIGN";
        case EQ: return "EQ";
        case LT: return "LT";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case TIMES: return "TIMES";
        case OVER: return "OVER";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case DOT: return "DOT";
        case COLON: return "COLON";
        case SEMI: return "SEMI";
        case COMMA: return "COMMA";
        case LMIDPAREN: return "LMIDPAREN";
        case RMIDPAREN: return "RMIDPAREN";
        case UNDERANGE: return "UNDERANGE";
        default: return "UNKNOWN";
    }
}

void initReservedWord(){
    reservedWords[0] = {"program", PROGRAM};
    reservedWords[1] = {"type", TYPE};
    reservedWords[2] = {"var", VAR};
    reservedWords[3] = {"procedure", PROCEDURE};
    reservedWords[4] = {"begin", BEGIN};
    reservedWords[5] = {"end", END1};
    reservedWords[6] = {"array", ARRAY};
    reservedWords[7] = {"of", OF};
    reservedWords[8] = {"record", RECORD};
    reservedWords[9] = {"if", IF};
    reservedWords[10] = {"then", THEN};
    reservedWords[11] = {"else", ELSE};
    reservedWords[12] = {"fi", FI};
    reservedWords[13] = {"while", WHILE};
    reservedWords[14] = {"do", DO};
    reservedWords[15] = {"endwh", ENDWH};
    reservedWords[16] = {"read", READ};
    reservedWords[17] = {"write", WRITE};
    reservedWords[18] = {"return", RETURN1};
    reservedWords[19] = {"integer", INTEGER};
    reservedWords[20] = {"char", CHAR};
}

// 遍历检查
bool isReserved(string ch)
{
    for (int i = 0; i < 21; i++)
        if (ch == reservedWords[i].str)
            return true;
    return false;
}

// 检查是不是字母
bool isLetter(char ch)
{
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        return true;
    else
        return false;
}

// 检查是不是数字
bool isDigit(char ch)
{
    if (ch >= '0' && ch <= '9')
        return true;
    else
        return false;
}

// 检查是不是单分界符
bool isOperator(char ch)
{
    if (ch == '+')
        return true;
    else if (ch == '-')
        return true;
    else if (ch == '*')
        return true;
    else if (ch == '/')
        return true;
    else if (ch == '<')
        return true;
    else if (ch == '=')
        return true;
    else if (ch == '(')
        return true;
    else if (ch == ')')
        return true;
    else if (ch == ';')
        return true;
    else if (ch == '[')
        return true;
    else if (ch == ']')
        return true;
    else if (ch == ',')
        return true;
    else
        return false;
}
//得到保留字数据
Sem getReserved(string s)
{
    for (int i = 0; i < 21; i++)
    {
        if (reservedWords[i].str == s)
            return reservedWords[i];
    }
}

// 特殊字符检查：空格 制表符 回车 换行
bool isBlank(char ch)
{

    if (ch == ' ') // 空格
        return true;
    else if (ch == '\n') // 换行
        return true;
    else if (ch == '\t')
        return true;
    else if (ch == '\r')
        return true;
    else
        return false;
}

int firstCharacter(char ch)
{
    switch (ch) {
        case ':':
            return 3;
        case '{':
            return 4;
        case '.':
            return 5;
        case '\'':
            return 6;
        default:
            if (isLetter(ch)) {
                return 0;
            }
            if (isDigit(ch)) {
                return 1;
            }
            if (isOperator(ch)) {
                return 2;
            }
            if (isBlank(ch)) {
                return 7;
            }
            return 8;
    }
}

void reserveWriteToken(int index, int lineNum, string record){
    tokenList[index].line = lineNum;
    tokenList[index].wd.str = getReserved(record).str;
    tokenList[index].wd.tok = getReserved(record).tok;
}

void writeToken(int index, int lineNum, string str, LexType temp_tok){
    tokenList[index].line = lineNum;
    tokenList[index].wd.str = str;
    tokenList[index].wd.tok = temp_tok;
}

// 直接转向法，自动机程序
void lexicalAnalyse(FILE* fp)
{
    int lineNum = 1;        //行号
    int index = 0;          // 标记token序号
    enum StateType state;  //存储状态机
    state = s0;

    char ch = fgetc(fp);
    string record = "";

    while (ch != EOF)
    {
        switch (state) {
            case s0:
            {
                record = "";
                switch (firstCharacter(ch)) {
                    case 0:state = s1; break;	//标识符状态
                    case 1:state = s2; break;  //数字状态
                    case 2:state = s3; break;  //单分界符状态
                    case 3:state = s4; break;	//:符状态
                    case 4:state = s6; break;  //{注释状态
                    case 5:state = s7; break;  //.状态
                    case 6:state = s9; break;  //\准字符状态
                    case 7:state = s12; break; //\r, \t, \n  ‘ ’
                    case 8:state = s13; break;  //出错状态
                }
            }break;

            case s1:{ //标识符状态
                if (isLetter(ch) || isDigit(ch)) {
                    //s1->s1
                    record = record + ch;
                    ch = fgetc(fp);
                }
                else {
                    if (isReserved(record)) // 检查是ID 还是保留字
                    {
                        reserveWriteToken(index, lineNum, record); // 保留字写入Token序列
                        index++;
                    }
                    else //标识符
                    {
                        writeToken(index, lineNum, record, ID);
                        index++;
                    }
                    state = s0; //回到初始状态
                }
            }break;
            case s2: {
                if (isDigit(ch) || isLetter(ch)) {
                    record = record + ch;
                    ch = fgetc(fp);
                }
                else {
                    bool flag = true;
                    for (int i = 0; i < record.length(); i++) {
                        if (isLetter(record[i])) // 只要有字母 就退出
                        {
                            flag = false;
                            tokenList[index].wd.tok = ERROR;
                            break;
                        }
                    }
                    if (flag) {
                        writeToken(index, lineNum, record, INTC);
                    }// INTC:无符号整数
                    index++;
                    state = s0;
                }
            }break;

            case s3:{ // char转string 运算符
                string temp = "";
                temp = temp + ch;
                if (temp == "+")
                    tokenList[index].wd.tok = PLUS;
                else if (temp == "-")
                    tokenList[index].wd.tok = MINUS;
                else if (temp == "*")
                    tokenList[index].wd.tok = TIMES;
                else if (temp == "/")
                    tokenList[index].wd.tok = OVER;
                else if (temp == "<")
                    tokenList[index].wd.tok = LT;
                else if (temp == "=")
                    tokenList[index].wd.tok = EQ;
                else if (temp == "(")
                    tokenList[index].wd.tok = LPAREN;
                else if (temp == ")")
                    tokenList[index].wd.tok = RPAREN;
                else if (temp == "[")
                    tokenList[index].wd.tok = LMIDPAREN;
                else if (temp == "]")
                    tokenList[index].wd.tok = RMIDPAREN;
                else if (temp == ";")
                    tokenList[index].wd.tok = SEMI;
                else if (temp == ",")
                    tokenList[index].wd.tok = COMMA;
                tokenList[index].line = lineNum;
                tokenList[index].wd.str = temp;
                index++;
                state = s0;
                ch = fgetc(fp);
            }break;

            case s4:{ //:状态
                record += ch;
                if ((ch = fgetc(fp)) == '=') //s5状态略去
                {
                    record += ch;
                    writeToken(index, lineNum,record, ASSIGN);
                    index++;
                    ch = fgetc(fp);
                }
                else
                {
                    writeToken(index, lineNum,record, COLON);
                    index++;
                }
                state = s0;
            }break;

            case s6:{  // {注释状态
                if (ch != '}') {  //特判{}
                    if (ch == '\n') { // 这是在干嘛？
                        lineNum += 1;
                    }
                }
                else {
                    state = s0;
                }
                ch = fgetc(fp);  //等待ending
            }break;

            case s7:{ //.状态
                record += ch;
                if ((ch = fgetc(fp)) == '.') // 代表了'..'
                {
                    record += ch;
                    writeToken(index,lineNum, record, UNDERANGE);
                    index++;
                    ch = fgetc(fp);
                }
                else // 记录'.'
                {
                    writeToken(index,lineNum, record, DOT);
                    index++;
                }
                state = s0;
            }break;
            case 9:{ // 字符常数状态 ，注意不是字符串！
                ch = fgetc(fp);
                if (isLetter(ch) || isDigit(ch) || ch == ' ') {
                    record += ch;
                    state = s10; //字符常量判断
                }
                else {
                    try {
                        throw IllegalIdentifier(lineNum, string (1,ch));
                    }catch (const IllegalIdentifier& e){
                        cout << e.what() << endl;
                        ifLexicalPass = false;
                        state = s0;
                    }
                }
            } break;

            case 10:{ // 紧接字符常量判断
                ch = fgetc(fp);
                if (ch == '\'') {
                    writeToken(index, lineNum, record, CHARC);
                    index++;
                    ch = fgetc(fp);
                }
                else {
                    try {
                        throw IllegalIdentifier(lineNum, string (1,ch));
                    }catch (const IllegalIdentifier& e){
                        cout << e.what() << endl;
                        ifLexicalPass = false;
                        state = s0;
                    }
                }
            }break;

            case 12:{
                if (ch == '\n')
                    lineNum += 1;
                state = s0;
                ch = fgetc(fp);
            }break;

            case 13:{
                try {
                    throw UnknownCharacter(lineNum);
                } catch (const UnknownCharacter& e) {
                    cout << e.what() << endl;
                    ifLexicalPass = false;
                    state = s0;
                    ch = fgetc(fp);
                    exit(1);
                }
            }break;
        }
    }
    writeToken(index, lineNum, "endfile", ENDFILE);
}

void printTokenList()
{
    int i = 0;
    cout  << 1 << endl; //重定向后莫名奇妙会吞一个end
    cout  << "编号"  << "\t" <<  std::left << setw(10) << "类型" << "\t\t" << std::left << setw(10) << "单词语义" << "\t\t" << std::left << setw(10) << "所在行数";
    cout << endl;
    while (tokenList[i].wd.tok != ENDFILE)
    {
        tokenList[i].index = i+1;
        cout << tokenList[i].index << "\t" << std::left << setw(10) << enumToString(tokenList[i].wd.tok)
             << "\t" << std::left << setw(10) << tokenList[i].wd.str <<"\t"<< tokenList[i].line << endl;
        i++;
    }
    cout << i
         << "\t" << std::left << setw(10) << enumToString(tokenList[i].wd.tok)
         << "\t" << std::left << setw(10) << tokenList[i].wd.str <<"\t" << tokenList[i].line << endl;
    i++;
}


