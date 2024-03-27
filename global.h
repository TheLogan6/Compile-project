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
    ENDWH, BEGIN,       END1,   READ , WRITE,
    ARRAY, OF,  RECORD, RETURN1,

    INTEGER, CHAR,
    /* 多字符单词符号 */
    ID, INTC, CHARC,
    /*特殊符号 */
    ASSIGN, EQ, LT, PLUS, MINUS,
    TIMES, OVER, LPAREN, RPAREN, DOT,
    COLON, SEMI, COMMA, LMIDPAREN, RMIDPAREN,
    UNDERANGE
};


enum IdKind
{
    typeKind,
    varKind,
    procKind
}; //标识符的类型

enum TypeKind{
    intTy,
    charTy,
    arrayTy,
    recordTy,
    boolTy
};  //内部类型

enum AccessKind
{
    dir,
    indir
}; //变量的类别。dir表直接变量(值参)，indir表示间接变量(变参)



extern jmp_buf jump_buffer;
