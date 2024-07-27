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
    string form;
    string name; //也是label
    string value;
    string access;
    int datalevel;
    int dataoff;
    Symbol* sym;

    Arg(){};
    Arg(string f, string n,string va="", string acc="indir", int dl=0, int df=0,Symbol* s = nullptr ) : \
        form(f),name(n),value(va), access(acc), datalevel(dl), dataoff(df),sym(s){};

    string getArgument(){
        if(form == "ValueForm"){
            return "ValueForm" + value;
        }
    }

};

class ActiveRecord{
public:
    string procname;
    int totalsize;
    int varistart = 0; //先
    int tempvaristart; //区域2
    int paramstart;    //区域3
    int statestart;   //状态信息4个：proc大小， 存储fp的位置， 返回值， 返回的地址
    int definelevel;

    ActiveRecord(string p, int total, int vari = 0, int tempvar = 0,int parm = 0, int st = 0, int del = 0):\
        procname(p), totalsize(total),varistart(vari), paramstart(parm), tempvaristart(tempvar),statestart(st), definelevel(del){};
};

class FourArgExp{
public:
    string codekind;
    int index;
    Arg* arg1;
    Arg* arg2;
    Arg* arg3;

    FourArgExp(string c, int in,  Arg* a1 = nullptr, Arg* a2 = nullptr, Arg* a3= nullptr): \
        codekind(c),index(in), arg1(a1),arg2(a2),arg3(a3){}


    string getfourArgExp(){
        string temp = to_string(index) + ":" + codekind + " ";
        if(arg1){
            if(arg1->form == "ValueForm"){
                temp += arg1->form + " " + arg1->name;
            }
            else{
                temp += " Arg1: " + arg1->form + " " + arg1->name + " " + arg1->access + " " + \
                to_string(arg1->datalevel) + " " + to_string(arg1->dataoff);
            }
            temp+= " ||| ";
        }
        if(arg2){
            temp += " Arg2: ";
            if(arg2->form == "ValueForm"){
                temp += arg2->form + " " + arg2->name;
            }
            else if(arg2->form == "AddrForm" || arg2->form == "TempForm"){
                temp +=  " " + arg2->form + " " + arg2->name + " " + arg2->access + " " + \
                to_string(arg2->datalevel) + " " + to_string(arg2->dataoff);
            }
            temp+= " ||| ";
        }
        if(arg3){
            temp +=  " Arg3: ";
            if(arg2->form == "ValueForm"){
                temp += arg2->form + " " + arg2->name;
            }
            else if(arg3->form == "AddrForm" || arg3->form == "TempForm"){
                temp += " " + arg3->form + " " + arg3->name + " " + arg3->access + " " + \
                to_string(arg3->datalevel) + " " + to_string(arg3->dataoff);
            }
        }
        return temp;
    }

    void beautPrint(){
        if(arg1 && arg2 && arg3){
            printf("(%s, %s, %s, %s)\n", codekind.c_str() , arg1->name.c_str(), arg2->name.c_str(), arg3->name.c_str());
        }
        else if(arg1 && arg2){
            printf("(%s, %s, %s, -)\n",codekind.c_str(), arg1->name.c_str(), arg2->name.c_str());
        }
        else if(arg1){
            printf("(%s, %s, -, -)\n",codekind.c_str(), arg1->name.c_str());
        }
        else {
            printf("(%s, -, -, -)\n",codekind.c_str());
        }
    }
};


//标签表
class labelTable{
public:
    map<string,int> labelList;


    void addToList(string Name, int index){
        labelList[Name] = index;
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


    int findArgListIndexByName(string tar);
    ActiveRecord* findActRedByName(string tar);
    void genRecordForMips(); //生成ActiveRecord
    void genCallChainMentry(); //调用链
    void genCallChainForMips(vector<ActiveRecord*> curact, int i);//递归过程生成


    void beautPrintExp();


};