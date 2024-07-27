#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <setjmp.h>


using namespace std;

//Vn
enum NonTerminal
{
    Program, ProgramHead, ProgramName, DeclarePart,
    TypeDec, TypeDeclaration, TypeDecList, TypeDecMore,
    TypeId, TypeName, BaseType, StructureType,
    ArrayType, Low, Top, RecType,
    FieldDecList, FieldDecMore, IdList, IdMore,
    VarDec, VarDeclaration, VarDecList, VarDecMore,
    VarIdList, VarIdMore, ProcDec, ProcDeclaration,
    ProcDecMore, ProcName, ParamList, ParamDecList,
    ParamMore, Param, FormList, FidMore,
    ProcDecPart, ProcBody, ProgramBody, StmList,
    StmMore, Stm, AssCall, AssignmentRest,
    ConditionalStm, StmL, LoopStm, InputStm,
    InVar, OutputStm, ReturnStm, CallStmRest,
    ActParamList, ActParamMore, RelExp, OtherRelE,
    Exp, OtherTerm, Term, OtherFactor,
    Factor, Variable, VariMore, FieldVar,
    FieldVarMore, CmpOp, AddOp, MultOp
};

// Vt
enum LexType
{
    ENDFILE, ERROR,
    /* 保留字 */
    PROGRAM, PROCEDURE, TYPE,   VAR, IF,
    THEN,   ELSE,       FI,     WHILE, DO,
    ENDWH, BEGIN,       END,   READ , WRITE,
    ARRAY, OF,  RECORD, RETURN,

    INTEGER, CHAR,
    /* 多字符单词符号 */
    ID, INTC, CHARC,
    /*特殊符号 */
    ASSIGN, EQ, LT, PLUS, MINUS,
    TIMES, OVER, LPAREN, RPAREN, DOT,
    COLON, SEMI, COMMA, LMIDPAREN, RMIDPAREN,
    UNDERANGE
};
typedef enum {ArrayK,CharK,IntegerK,RecordK,IdK}  DecKind;
typedef enum {IfK,WhileK,AssignK,ReadK,WriteK,CallK,ReturnK} StmtKind;
typedef enum {OpK,ConstK,VariK} ExpKind;           //表达式类型分为操作符类型：a+b 常整数类型：6 变量类型：a
typedef enum {IdV,ArrayMembV,FieldMembV} VarKind; //标识符变量 数组成员变量 域成员变量
typedef enum {Void,Integer,Boolean} ExpType;      //表达式整个节点的检查类型（为语义分析判别打基础
typedef enum {none,varparamType,valparamType} ParamType;  //参数类型，val和var


