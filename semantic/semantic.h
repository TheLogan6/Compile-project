#pragma once
#include <iostream>
#include <string>
#include "global.h"
#include "exception.h"
#include "lexical.h"
#include "grammar.h"

using namespace std;

class Symbol;
class SymbolTable;
class GrammarTree;
class type;
class TreeNode;
class basetype;
class arraytype;
class recordtype;


//语法节点的！
typedef enum {ProK,PheadK,TypeK,VarK,ProcDecK,StmLK,DecK,StmtK,ExpK}NodeKind;
extern string nodeKindToString(NodeKind x);




class basetype{
public:
    basetype(int s= 4, string tn = ""):type_name(tn){
        size =  s;
    }
public:
    int size;
    string type_name; //integer or char
};

class arraytype{
public:
    int low, top;
    string type_name = "arrayType";
    basetype* element;
    int size;

    arraytype(int l, int t, basetype* e){
        low = l;
        top = t;
        element  = e;
        assert(e != nullptr);
        size = (t - l + 1)*(e->size);
    }
};

class recordtype{
public:
    recordtype(int s, SymbolTable* fi= nullptr ):filedList(fi), size(s){}
    string type_name = "recordType";
    SymbolTable* filedList;
    int size;
};

class type{
public:
    basetype* bt;
    arraytype* at;
    recordtype* rt;

    type(){
        bt = nullptr;
        at = nullptr;
        rt = nullptr;
    }

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
            return rt->size;
        }
    }
};

void printSymbolTable(SymbolTable* symtbl);



class Symbol{
public:
    string name;
    NodeKind decKind;   //声明类型,typeDec,varDec,procDec
    type* tp;
    Token* tk;

    string access;
    int level, offset;

    SymbolTable* param;
    int code;
    int size;
    int paramsize;
    int tempvarisize;

    Symbol(string n, NodeKind d, type* p, Token* tk = nullptr):name(n),decKind(d),\
        tp(p), tk(tk){ access = "dir"; code = 0;size = 0;paramsize = 0;tempvarisize = 0;};

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

    string getName(){
        return name;
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
        for(int i = 0; i < level; i++){
            cout<< '\t';
        }
        cout << "Name:" << name << "  decKind:" << nodeKindToString(decKind) << \
        "  Type:" << getType();
//        "  TokenStr:" << tk->wd.str
        if(decKind == TypeK){
            cout << endl;
        }
        else if(decKind == VarK){
            cout << '\t'  << " access:"  << access << "  level:" << level << "  offset:" << offset << endl;
        }
        else if(decKind == ProcDecK){
            cout << '\t' << " level:" << level << "  offset:" << offset << endl;
        }
        if(getType() == "RecordType"){
            for(int i = 0; i < level; i++) cout<< '\t';
            cout << "***进入Record内部***" <<endl;
            SymbolTable* tmp = this->tp->rt->filedList;
            printSymbolTable(tmp);
        }

    }
};

class SymbolTable //这是一个链表结构
{
public:
    vector<Symbol*> table;
    void addSymbol(Symbol* sym){ //重载一下
        table.push_back(sym);
    }
//    void addSymbol(string name, NodeKind d, type* p, Token* tk, SymbolTable* next = NULL, int dir = 1)
//    {
//        table.push_back(new Symbol(name, d, p, tk));
//    }

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
    void printTable();

};//符号表


class Analyzer{
public:

    Token* tokenList;
    GrammarTree* tree;

    vector<SymbolTable*> scope;
    SymbolTable* tbSym; //当前level的符号表
    int levelCurrent;
    vector<int> offsetCurrent;

    vector<string> runMessgae;
    vector<Symbol*> semanticList;//为中间代码生成铺垫的

    Analyzer(Token token[1000], GrammarTree* t):tokenList(token), tree(t){
        tbSym = new SymbolTable();
        scope.push_back(tbSym);
        levelCurrent = -1;
        offsetCurrent.resize(0);
        offsetCurrent.clear();
        runMessgae.clear();
    }
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

    Symbol* synaxExpression();//存在递归调用，返回具体载体
    Symbol* synaxTerm();
    Symbol* synaxFactor();
    Symbol* synaxVariable();
    Symbol* synaxRelExpression();

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

    //为目标代码生成铺垫
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
                    if(t->table[k]->decKind == TypeK) continue;
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


