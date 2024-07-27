#pragma once
#include <iostream>
#include <string>
#include "global.h"
#include "exception.h"
#include "lexical.h"
#include "grammar.h"
#include "midcode.h"

enum RegType{$zero = 0, $at, $v0, $v1, $a0,$a1,$a2,$a3, $t0,$t1,$t2,$t3,$t4,$t5,$t6,$t7,
    $s0, $s1,$s2,$s3,$s4,$s5,$s6,$s7, $t8,$t9, $k0,$k1,$gp, $sp, $fp,$ra  };

class Register{
public:
    RegType reg;
    bool used;
    Arg* target;

    Register(){}
    Register(RegType r, bool u = false, Arg* t = nullptr):reg(r),used(u),target(t){};

};

class RegManagement{//寄存器管理器
public:
    Register regfile[31];
    int lruqueue[24];  //0-23的位置 存储了 1-24号reg

    RegManagement(){
        for(int i = 0; i <= 31; i++){
            regfile[i] = Register(static_cast<RegType>(i), false, nullptr);
        }
        regfile[0].used = true;
        for(int i = 0; i <= 23; i++){
            lruqueue[i] = i+1; //t8是24号是t8， 尽量也别用
        }
    };


    string regtypeToString(RegType x);
    string allocateReg(Arg* arg,string stmType="Default");
    void lruRecentUse(int use){
        bool find = false;
        int pos = -1;
        for(int i = 0; i <= 23; i++){
            if(lruqueue[i] == use){
                find = true;
                pos = i;
                break;
            }
        }
        if(find == false){
            assert(false);
        }
        for(int i = pos; i < 23; i++){
            lruqueue[i] = lruqueue[i+1];
        }
        lruqueue[23] = use;
    }
};


class FinalCodeGen{
public:
    FinalCodeGen(vector<FourArgExp*> argE,vector<SymbolTable*> s,vector<ActiveRecord*> act,vector<vector<ActiveRecord*>> callo ):\
        argExpList(argE),scope(s), actrecord(act), regmmu(), callorder(callo){
            callorder = callo;
            mipsCode.resize(0);
            mipsCode.clear();
            curgenPro = nullptr;
    };

    vector<FourArgExp*> argExpList;
    vector<SymbolTable*> scope;//scope栈就是类似声明链了

    vector<ActiveRecord*> actrecord; //声明栈
    vector<vector<ActiveRecord*>> callorder; //这是调用链，调用链也代表内存会出现的可能状况
    RegManagement regmmu;

//    int CurCallChainIndex = 0;
    vector<string> mipsCode;
    Symbol* curgenPro;
    ActiveRecord* curgenRecord;

    void finalCodeGen();


    Symbol* findByname(string target){
        int m = scope.size();
        for(int i = m-1; i >= 0; i--)
        {
            SymbolTable* stbl = scope[i];
            int symnum = stbl->table.size();
            for(int i = 0; i < symnum; i++){
                if(stbl->table[i]->name == target){
                    return stbl->table[i];
                }
            }
        }
        return nullptr;
    }

    string parseArgAddress(Arg* arg);

    ActiveRecord* findRecordByName(string target){
        int n = actrecord.size();
        for(int i = 0; i < n; i++){
            if(actrecord[i]->procname == target){
                return actrecord[i];
            }
        }
        return nullptr;
    }

    //mips生成区
    string mipsgenArith_R(string op, string rd, string rs, string rt);
    string mipsgenArith_I(string op, string rt, string rs, string imme);
    string mipsgenMem_I(string op, string rt, string rs, string off);
    string mipsgenMemIndir(string op, string rt, string rs);
    string mipsgenJump(string op, string label);
    string mipsgenBranchZ(string op,string rs, string rt, string label);
    string mipsgenLabel(string label);
    string mipsgenNop();

    void mipsgenStoreProcState();

    string findOffsetByArg(Arg* arg1);

    Symbol* findFromScopeByProc_Name(string procname, string symname);

};