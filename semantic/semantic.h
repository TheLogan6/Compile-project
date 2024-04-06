#pragma once
#include <iostream>
#include <string>
#include "global.h"
#include "exception.h"
#include "lexical.h"
#include "grammar.h"

using namespace std;

class SymbolTable;
class type;
class TreeNode;
class basetype;
class arraytype;
class recordtype;

struct SymbolRecNode;
struct SymbolRec;
extern SymbolTable* smbltable;

//这还是用来操作 语法节点的！
typedef enum {ProK,PheadK,TypeK,VarK,ProcDecK,StmLK,DecK,StmtK,ExpK}NodeKind;

extern string nodeKindToString(NodeKind x);


typedef enum {ArrayK,CharK,IntegerK,RecordK,IdK}  DecKind;
typedef enum {IfK,WhileK,AssignK,ReadK,WriteK,CallK,ReturnK} StmtKind;
typedef enum {OpK,ConstK,VariK} ExpKind;           //表达式类型分为操作符类型：a+b 常整数类型：6 变量类型：a
typedef enum {IdV,ArrayMembV,FieldMembV} VarKind; //标识符变量 数组成员变量 域成员变量
typedef enum {Void,Integer,Boolean} ExpType;      //表达式整个节点的检查类型（为语义分析判别打基础
typedef enum {none,varparamType,valparamType} ParamType;  //参数类型，val和var

class basetype{
public:
    basetype(int s= 1, string tn = ""):type_name(tn){
        size =  s;
    }
public:
    int size;
    string type_name; //integer or char
};

class arraytype{
public:
    arraytype(int l, int t, basetype* e){
        low = l;
        top = t;
        element  = e;
        assert(e != nullptr);
        size = (t - l + 1)*(e->size);
    }
    int low, top;
    string type_name = "arrayType";
    basetype* element;
    int size;
};
class recordtype{
public:
    int size;
};

class type{
public:
    type(){
        bt = nullptr;
        at = nullptr;
        rt = nullptr;
    }
    basetype* bt;
    arraytype* at;
    recordtype* rt;
    string getType(){
        if(bt){
            return bt->type_name;
        }
        else if(at){
            return at->element->type_name;
        }
        else if(rt){
            return "RecordType";
        }
    }

    int getSize(){
        if(bt){
            return bt->size;
        }
        else if(at){
            return (at->top - at->low + 1) * (at->element->size);
        }
        else if(rt){
            assert(false);
        }
    }
};


class Symbol{
public:
    Symbol(string n, NodeKind d, type* p, Token* tk = nullptr):name(n),decKind(d),\
        tp(p), tk(tk){ access = "dir"; code = 0;size = 0;paramsize = 0;tempvarisize = 0;};

    string name;
    NodeKind decKind; //声明类型,typeDec,varDec,procDec
    type* tp;
    Token* tk;
    SymbolTable* next;  //这是针对Prok，只有在ProK时生效

    string access;
    int level, offset;
    int value;

    SymbolTable* param;
    int _class;
    int code;
    int size;
    int paramsize;
    int tempvarisize;
    //int forward;  //proc专属

    void varSymbolCons(string acc, int lev, int off){
        access = acc;
        level = lev;
        offset = off;
        if(tp){
            size = this->getSymbolSize();
        }
    }

    void procSymbolCons(SymbolTable* par = nullptr, int lev = 0, int off = 0){
        param = par;
        level = lev;
        offset = off;
    }

    string getType(){
        if(decKind == ProcDecK) return "ProcKType";
        if(tp)
        {
            if(tp->bt) {
                return tp->bt->type_name;  //要具体一点的
            }
            else if(tp->at) return "ArrayType";
            else if(tp->rt) return "RecordType";
        }
        else return name;
    }
    string getTillBaseType(){
        if(decKind == ProcDecK) return "ProcKType";
        if(tp)
        {
            if(tp->bt) {
                return tp->bt->type_name;  //要具体一点的
            }
            else if(tp->at){
                return tp->at->element->type_name;
            }
            else if(tp->rt) {
                return "RecordType";
            }
        }
        else return name;
    }

    int getSymbolSize(){
        if(tp->bt){
            return tp->bt->size;
        }
        else if(tp->at){
            return tp->at->size;
        }
        else if(tp->rt){
            return tp->rt->size;
        }
    }

    void printSymbol(){
        //基础类型
        cout << "Name:" << name << "  decKind:" << nodeKindToString(decKind) << \
        "  Type:" << getType() <<  "  TokenLexType: " << enumToString(tk->wd.tok)  << endl;
//        "  TokenStr:" << tk->wd.str
        if(decKind == VarK){
            cout << '\t'  << "VarSymbolExtra : access:"  << access << "  level:" << level << "  offset:" << offset << endl;
        }
        else if(decKind == ProcDecK){
            cout << '\t' << "ProcSymbolExtra  level:" << level << "  offset:" << offset << endl;
        }
        if(tp)
        {
            if(tp->at){

            }
            else if(tp->rt){

            }
        }

    }
};

typedef struct SymbolTable //这是一个链表结构
{
//    string idname;
//    AttributeIR attrIR;
    vector<Symbol*> table;
//    int paramcount;
//    SymbolTable* pre;//scope栈维护方式
    void addSymbol(Symbol* sym){
        table.push_back(sym);
    }

    void addSymbol(string name, NodeKind d, type* p, Token* tk, SymbolTable* next = NULL, int dir = 1)
    {
        table.push_back(new Symbol(name, d, p, tk));
//        table[index]->next = next;
//        if(next){
//            next->pre = this;
//        }
//        index++;
    }

    Symbol* findByName(string target){
        int n = table.size();
        Symbol* p = nullptr;
        for(int i = 0; i < n; i++){
            if(table[i]->name ==  target){
                return table[i];
            }
        }
        return nullptr;
    }
}SymbTable;//符号表
class GrammarTree;

class Analyzer{
public:
    Analyzer(Token token[1000], GrammarTree* t):tokenList(token), tree(t){
        tbSym = new SymbolTable();
        scope.push_back(tbSym);
        levelCurrent = -1;
        offsetCurrent.resize(0);
        offsetCurrent.clear();
        runMessgae.clear();
    }

    vector<SymbolTable*> scope;
    SymbolTable* tbSym;
    int levelCurrent;
    vector<int> offsetCurrent;

    Token* tokenList;
    GrammarTree* tree;

    vector<string> runMessgae;
    vector<Symbol*> semanticList;

    void semanticAnalyze();
    void initSemantic();
    void grammarToSynax();

    void synaxDeclareAll();
    void synaxTypeDec();
    void synaxVarDec();
    void synaxProDec();

    void synaxProgramAnalyze();
    void synaxStmList();
    void synaxConditionalStm();
    void synaxLoopStm();
    void synaxInputStm();
    void synaxOutputStm();
    void synaxReturnStm();

    Symbol* synaxExpression();
    Symbol* synaxTerm();
    Symbol* synaxFactor();
    Symbol* synaxVariable(); //需要同时返回 “文法类型” & “词法具体值”
    Symbol* synaxRelExpression();  //条件判断分析

    bool assign_typecheck(string lefttype, string righttype);


    type* getIdType();
    type* getBaseType();
    type* getArrayType();

    void printmessage(){
        int nummes = runMessgae.size();
        for(int i = 0; i < nummes; i++){
            cout << runMessgae[i] << endl;
        }
    }

    void getSemanticList(TreeNode* p);

    void calcuProcSize(){
        int m = scope.size();
        for(int i = 0; i < m; i++){ //遍历scope 每一个scope是一个函数
            SymbolTable* t = scope[i];
            int n = t->table.size();
            Symbol* curpro = t->table[0];//一定时
            if(curpro->decKind == ProcDecK)
            {
                for(int k = 1; k < n; k++)
                {
                    if(t->table[k]->decKind != ProcDecK){
                        curpro->size += t->table[k]->getSymbolSize();
                    }
                    else {
                        SymbolTable* parmtbl = t->table[k]->param;
                        int parmnum = parmtbl->table.size();
                        for(int l = 0; l < parmnum; l++){
                            curpro->size += parmtbl->table[l]->getSymbolSize();
                            curpro->paramsize += parmtbl->table[l]->getSymbolSize();
                        }
                    }
                }
            }
            else assert(false);
        }
    }
};


//typedef struct //标识符的属性结构定义
//{
//    struct TypeIR* idtype;	//指向标识符的类型内部表示 pro，var type都需要
//    IdKind kind;			//标识符的类型
//    union
//    {
//        struct
//        {
//            AccessKind access;   //是否直接获得
//            int level;
//            int off;
////            bool isParam;  //判断是参数还是普通变量
//        }VarAttr;                             //变量标识符的属性
//        struct
//        {
//            int level;     //该过程的层数
//            ParamTable* param;   //参数表
//            int size;
//            int code;
//        }ProcAttr;                            //过程名标识符的属性
//    }More;//标识符的不同类型有不同的属性
//}AttributeIR;                   //标识符信息项


