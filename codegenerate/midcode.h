#pragma once
#include <iostream>
#include <string>
#include <map>
#include "global.h"
#include "exception.h"
#include "lexical.h"
#include "grammar.h"
class Arg{
public:
    Arg(){};
    Arg(string f, string n,string va="", string acc="indir", int dl=0, int df=0,Symbol* s = nullptr ) : \
        form(f),name(n),value(va), access(acc), datalevel(dl), dataoff(df),sym(s){};
    string form;
    string name; //也是label
    string value;
    string access;
    int datalevel;
    int dataoff;
    Symbol* sym;

    string getArgument(){
        if(form == "ValueForm"){
            return "ValueForm" + value;
        }
    }

};

class ActiveRecord{
public:
    ActiveRecord(string p, int total, int vari = 0, int tempvar = 0,int parm = 0, int st = 0, int del = 0):\
        procname(p), totalsize(total),varistart(vari), paramstart(parm), tempvaristart(tempvar),statestart(st), definelevel(del){};
    string procname;
    int totalsize;
    int varistart = 0; //先
    int tempvaristart; //then
    int paramstart;    //then
    int statestart;   //状态信息4个：proc大小， 存储fp的位置， 返回值， 返回的地址
    int definelevel;
};

class FourArgExp{
public:
    FourArgExp(string c, int in,  Arg* a1 = nullptr, Arg* a2 = nullptr, Arg* a3= nullptr): \
        codekind(c),index(in), arg1(a1),arg2(a2),arg3(a3){}
    string codekind;
    int index;
    Arg* arg1;
    Arg* arg2;
    Arg* arg3;


    string getfourArgExp(){
        string temp = to_string(index) + ":" + codekind + " ";
        if(arg1){
            temp += " Agr1: " + arg1->form + " " + arg1->name + " " + arg1->access + " " + \
                to_string(arg1->datalevel) + " " + to_string(arg1->dataoff);
        }
        if(arg2){
            temp += " Agr2: " + arg2->name;
            if(arg2->form == "AddrForm" || arg2->form == "TempForm"){
                temp += '\n' + "Extra Arg2:" + arg2->form + " " + arg2->name + " " + arg2->access + " " + \
                to_string(arg2->datalevel) + " " + to_string(arg2->dataoff);
            }
        }
        if(arg3){
            temp +=  " Agr3: " + arg3->name;
            if(arg3->form == "AddrForm" || arg3->form == "TempForm"){
                temp += '\n' + "Extra arg3:" + arg3->form + " " + arg3->name + " " + arg3->access + " " + \
                to_string(arg3->datalevel) + " " + to_string(arg3->dataoff);
            }
        }
        return temp;
    }
};


//标签表
class labelTable{
public:
    map<string,int> labelList;
    int labelLen;

    void addToList(string Name, int index){
        labelList[Name] = index;
    }

    void findLabelByName(string s){

    }
};


class midCodeGenerate{
public:
    midCodeGenerate(GrammarTree* t, Token* tl, int tvg = 0, int lg = 0): \
       tree(t),tokenList(tl),tempvargen(tvg),labelgen(lg){
        labeltlb = labelTable();
        argExpList.resize(0);
        argExpList.clear();
        curanalysispro = nullptr;
    }
    GrammarTree* tree;
    Token* tokenList;
    int tempvargen = 0;
    int labelgen = 0;
    int codeindex = 0;
    vector<Symbol*> prostack;  //过程调用栈
    labelTable labeltlb;
    vector<FourArgExp*> argExpList;
    Symbol* curanalysispro;
    vector<ActiveRecord*> actrecord;  //注意定义顺序是倒着的！！先begin-end的act在前面
    vector<vector<ActiveRecord*>> callorder;

    void grammarTreeScan();
    void codeAnalysisDeclare();
    void codeAnalysisProgramBody();

    void midgenProDec();
    void midgenStmList();
    void midgenConditionalStm();
    void midgenLoopStm();
    void midgenInputStm();
    void midgenOutputStm();
    void midgenReturnStm();

    Arg* midgenExpression();
    Arg* midgenTerm();
    Arg* midgenFactor();
    Arg* midgenVariable();
    Arg* midgenBoolExp();
    Arg* midgenArray(Symbol* arrayid);

    Arg* midgenExpCode(string op, Arg* left, Arg* right);
    void midgenAssign(Arg* left, Arg* right);
    Arg* midgenBoolCode(string op, Arg* left, Arg* right);
    void midgenJumpCode(string op, Arg* boolarg, Arg* elsearg);
    void midgenJumpCode(string op, Arg* boolarg); //重载
    void midgenLabelCode(string op, Arg* labelarg);
    void midgenProcedureCallCode(vector<Arg*> param, Symbol* proc);
    void midgenParamCode(string op, Arg* arg, int size, int off);
    void midgenProcEndCode();
    void midgenReturnCode(string op);


    string symbolToWord(string op);
    void curProAddTempVari(Arg* tmparg);

    void genRecordForMips();
    void genCallChainMentry();
    int findArgListIndexByName(string tar);
    void genCallChainForMips(vector<ActiveRecord*> curact, int i);
    ActiveRecord* findActRedByName(string tar);

};