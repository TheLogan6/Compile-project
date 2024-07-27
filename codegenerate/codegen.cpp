#pragma GCC optimize("O0")
#include "codegen.h"
#include <fstream>
#include <cctype>
using namespace std;

extern Token tokenList[1010];
extern int pointer; //treepointer
extern GrammarTree* tree;


#define LEXOPENFILEDIR "../Reference/test9bubble.txt"
#define LEXOUTPUTFILEDIR "../Output/lexicaloutput.txt"

#define GRAOUTPUTFILEDIR "../Output/test9gra.txt"
#define SEMANOUTPUTFILEDIR "../Output/test9semantic.txt"
#define MIDCODEOUTPUTDIR "../Output/test9mid.txt"
#define MIPSCODEOUTPUTDIR "../Output/test9mipscode.txt"

#define GRAMMARMAIN
#define SEMANTICMAIN
#define MIDCODEMAIN
//#define MIPSCODEMAIN
#define MIPSDEBUG 1
#define NITIAN 0

#ifdef MIPSCODEMAIN

int main(){
    initReservedWord(); //初始化保留字
    FILE *fp;
    std::ofstream outputFile(MIDCODEOUTPUTDIR, std::ios::out | std::ios::binary);
    if(outputFile.is_open()){
        outputFile << u8"\uFEFF"; // 写入 BOM (Byte Order Mark)，表示文件以 UTF-8 编码
    }

    fp = fopen(LEXOPENFILEDIR, "r");
    fflush(stdout);
    lexicalAnalyse(fp);
//    printTokenList();
    TreeNode* t = program();
    tree = new GrammarTree(0, t, t);

#ifdef GRAMMARMAIN
     freopen(GRAOUTPUTFILEDIR, "w", stdout); // 输出重定向
    cout << "Part2 : Grammar Analysis" << endl;  //奇怪的重定问题
    if (tokenList[pointer].wd.tok == ENDFILE) {
        cout << "grammar endfile !" << endl;
    }
    tree->dfsPrintTree(t,0);
//    tree->printToPy(t, 0);
#endif

#ifdef SEMANTICMAIN
    freopen(SEMANOUTPUTFILEDIR, "w", stdout); // 输出重定向
    cout << "Part3 : Semantic Analysis" << endl;  //奇怪的重定问题
    Analyzer analyzer = Analyzer(tokenList, tree);
    analyzer.semanticAnalyze();
    analyzer.tree->goRoot();
//    analyzer.getSemanticList(analyzer.tree->root);
#endif
#ifdef MIDCODEMAIN
    freopen(MIDCODEOUTPUTDIR, "w", stdout); // 输出重定向
    cout << "Part4 : Mid code generate!" << endl;  //奇怪的重定问题
    midCodeGenerate midgen = midCodeGenerate(tree,tokenList,0,0);
    midgen.grammarTreeScan();

#endif
#ifdef MIPSCODEMAIN
    freopen(MIPSCODEOUTPUTDIR, "w", stdout);
    cout << "# Part5 : MIPS code generate!" << endl;  //奇怪的重定问题
    FinalCodeGen mipsgen(midgen.argExpList, analyzer.scope, midgen.actrecord,midgen.callorder);
    mipsgen.finalCodeGen();
#endif
    fclose(fp);
}
#endif



void FinalCodeGen::finalCodeGen() {
    //全局变量声明
    int m = argExpList.size();
    mipsgenJump("j", "main");
    for(int i = 0; i < m; i++)
    {
        FourArgExp* curexp = argExpList[i];
        string codechoice = curexp->codekind;
        if(MIPSDEBUG) cout << "#"<< argExpList[i]->index << codechoice << endl;
        if(codechoice == "ASSIGN"){   //arg1 = arg2
            //先处理arg2
            string ldreg2,off2; //需要store的值
            ldreg2 = regmmu.allocateReg(curexp->arg2);
            if(curexp->arg2->form == "ValueForm"){
//                off2 = parseArgAddress(curexp->arg2);
                mipsgenArith_R("addi", ldreg2, "$zero", curexp->arg2->name);
            }
            else if(curexp->arg2->form == "TempForm" || curexp->arg2->form == "AddrForm"){
                if(curexp->arg2->access == "indir"){
                    off2 = parseArgAddress(curexp->arg2);
                    string ldregindir = regmmu.allocateReg(nullptr,"OnlyOnce");
                    mipsgenMem_I("lw", ldregindir, "$sp", off2);
                    mipsgenArith_R("add",ldregindir, ldregindir, "$sp");
                    mipsgenMem_I("lw", ldreg2, ldregindir, to_string(0));
                }
                else{
                    off2 = parseArgAddress(curexp->arg2);
                    mipsgenMem_I("lw",ldreg2,"$sp",off2);
                }
            }
            //再处理arg1
            string off1;   //arg1不需要分配直接ld的地方
            if(curexp->arg1->access == "dir"){
                off1 = parseArgAddress(curexp->arg1);
                mipsgenMem_I("sw", ldreg2 ,"$sp",off1);
            }
            else if(curexp->arg1->access == "indir"){
                off1 = parseArgAddress(curexp->arg1);
                string ldregindir = regmmu.allocateReg(nullptr, "OnlyOnce");
                mipsgenMem_I("lw", ldregindir, "$sp", off1);
                mipsgenArith_R("add",ldregindir, ldregindir, "$sp");
                mipsgenMem_I("sw", ldreg2, ldregindir , to_string(0));
            }
        }

        else if(codechoice == "ADD" || codechoice =="SUB" || codechoice =="MULT" || codechoice =="DIV" || codechoice == "LTC"){
            //arg3 = arg1+arg2
            string off1, ldreg1;
            ldreg1 = regmmu.allocateReg(curexp->arg1);
            if(curexp->arg1->form == "ValueForm"){
                mipsgenArith_R("addi", ldreg1, "$zero", curexp->arg1->name);
            }
            else if(curexp->arg1->form == "TempForm" || curexp->arg1->form == "AddrForm"){//mmd
                off1 = parseArgAddress(curexp->arg1);
                mipsgenMem_I("lw",ldreg1,"$sp", off1);
                if(curexp->arg1->access == "indir"){
                    string ldregindir1 = regmmu.allocateReg(nullptr,"OnlyOnce");
                    mipsgenArith_R("add",ldregindir1, "$sp", ldreg1);
                    mipsgenMem_I("lw", ldreg1, ldregindir1, to_string(0));
                }
            } //处理完Arg1
            string off2, ldreg2;
            ldreg2 = regmmu.allocateReg(curexp->arg2);
            if(curexp->arg2->form != "ValueForm")
                if(NITIAN) {
                    cout << "不是ValueForm" <<endl;
                    cout << curexp->getfourArgExp() << endl;
                    cout <<curexp->arg2->form << curexp->arg2->name << curexp->arg2->dataoff << "逆天错误" << endl;
                }
            if(curexp->arg2->form == "ValueForm"){
                mipsgenArith_R("addi", ldreg2, "$zero", curexp->arg2->name);
            }
            else if(curexp->arg2->form == "TempForm" || curexp->arg2->form == "AddrForm"){
                off2 = parseArgAddress(curexp->arg2);
                mipsgenMem_I("lw",ldreg2,"$sp", off2);
                if(curexp->arg2->access == "indir"){
                    string ldregindir2 = regmmu.allocateReg(nullptr,"OnlyOnce");
                    mipsgenArith_R("add",ldregindir2, "$sp", ldreg2);
                    mipsgenMem_I("lw", ldreg2, ldregindir2, to_string(0));
                }
            }//处理完Arg2
            string ldreg3, off3;
            ldreg3 = regmmu.allocateReg(curexp->arg3);
            off3 = parseArgAddress(curexp->arg3);
            string op = codechoice;
            if(codechoice == "LTC")
                op = "slt";
            else if(codechoice == "MULT")
                op = "mul";
            mipsgenArith_R(op, ldreg3, ldreg1, ldreg2);

            mipsgenMem_I("sw", ldreg3, "$sp", off3);
        }
        else if(codechoice == "AADD"){
            string offarraybase, off2, off3;
            string ldreg_offset;
            offarraybase = parseArgAddress(curexp->arg1);
            off2 = parseArgAddress(curexp->arg2);
            ldreg_offset = regmmu.allocateReg(curexp->arg2);
            mipsgenMem_I("lw", ldreg_offset, "$sp", off2);
            mipsgenArith_I("addi", ldreg_offset, ldreg_offset, offarraybase);
            off3 = parseArgAddress(curexp->arg3);
            mipsgenMem_I("sw", ldreg_offset, "$sp", off3);
        }
        else if(codechoice == "Label"){
            mipsgenLabel(curexp->arg1->name);
        }
        else if(codechoice == "EQC"){//mips没有这类指令啊
            assert(false);
        }
        else if(codechoice == "JUMP"){
            mipsgenJump("j", curexp->arg1->name);
        }
        else if(codechoice == "JUMP0"){//等于0则跳转
            string ldreg,off;
            ldreg = regmmu.allocateReg(curexp->arg1);
            off = parseArgAddress(curexp->arg1);
            mipsgenMem_I("lw", ldreg, "$sp", off);
            mipsgenBranchZ("beq", ldreg, "$zero", curexp->arg2->name);
        }
        else if(codechoice == "READC"){
            Arg* readt = argExpList[i]->arg1;
            assert(readt->form == "AddrForm");
            string off1 = parseArgAddress(readt);
            string ldreg1 = regmmu.allocateReg(curexp->arg1);
            mipsgenMem_I("lw",ldreg1, "$sp", off1);
        } //load一下寄存器而已
        else if(codechoice == "MENTRY"){
            cout << endl;
            //main. mov fp sp , addi sp sp -30 , sw fp ()sp ,  sw ra ()sp
            Arg* curprocarg = argExpList[i]->arg1;
            curgenPro = curprocarg->sym;
            curgenRecord = findRecordByName(curprocarg->name);
            mipsgenLabel("main");
            mipsgenStoreProcState();
            if(MIPSDEBUG) cout << "#状态end" << endl;
        }
        else if(codechoice == "PENTRY"){
            cout << endl;
            Arg* curprocarg = argExpList[i]->arg1;
            curgenPro = curprocarg->sym;
            curgenRecord = findRecordByName(curprocarg->name);
            mipsgenLabel(curprocarg->name); //proc:
            mipsgenStoreProcState();
            if(MIPSDEBUG) cout << "#状态end" << endl;
            //调用者才需要参数赋值
            int paramsize = 0;
            bool isFindPro = false;
            int tarcall = 0;
            int argument_offstart = 0;
            for(int i = 0; i < m; i++){
                vector<ActiveRecord*> curchain = callorder[i];
                int chainnum = curchain.size();
                for(int j = 0; j < chainnum ; j++){//调用顺序
                    if(curchain[j]->procname == curgenPro->name){ //根据procname找sym是否在其中
                        isFindPro = true;
                        tarcall = j;
                        break;
                    }
                }
                if(isFindPro){
                    argument_offstart = curchain[tarcall-1]->paramstart + curchain[tarcall]->totalsize;
                    paramsize = (curchain[tarcall-1]->statestart - curchain[tarcall-1]->paramstart);
                    break;
                }
            }
            for(int k = 0; k < paramsize; k+=4)
            {
                string lgregtemp = regmmu.allocateReg(nullptr,"OnlyOnce");
                mipsgenMem_I("lw", lgregtemp ,"$sp", to_string(argument_offstart));
                argument_offstart += 4;
                mipsgenMem_I("sw", lgregtemp, "$sp", to_string(k));
            }
            if(MIPSDEBUG) cout << "#参数加载" << endl;
        }
        else if(codechoice == "CALL"){
            Arg* curprocarg = argExpList[i]->arg1;
            mipsgenJump("jal", curexp->arg1->name);

        }
        else if(codechoice == "VALACT" || codechoice == "VARACT"){
            string off,ldreg;
            off = parseArgAddress(curexp->arg1);
            ldreg = regmmu.allocateReg(curexp->arg1);
            mipsgenMem_I("lw",ldreg, "$sp", off);
            int parampoint = curgenRecord->paramstart + stoi(curexp->arg3->name)*4;
            mipsgenMem_I("sw",ldreg,"$sp", to_string(parampoint));
        }
        else if(codechoice == "ENDPROC"){
            if(MIPSDEBUG) cout << "#开始退出" << endl;
            mipsgenArith_I("addiu","$sp", "$fp", "0");
            mipsgenMem_I("lw", "$fp", "$sp", to_string((curgenRecord->statestart+4)));//*4
            mipsgenArith_I("addi", "$sp", "$sp", to_string(curgenRecord->totalsize));//*4
            if (curgenRecord->procname != callorder[0][0]->procname)
                mipsgenJump("jr", "$ra");
            mipsgenNop();
            if(MIPSDEBUG) cout << "#函数end" <<endl;
        }
    }
}

string RegManagement::allocateReg(Arg* arg, string stmType) {
    bool existEmpty = false;
    string targetgre;
    if(stmType == "OnlyOnce"){
        int i = 25;
        targetgre = regtypeToString(static_cast<RegType>(i));
        return targetgre;
    }
    //记录已分配
    if(arg->form == "TempForm" || arg->form == "AddrForm"){ //不是value时才可以找
        for(int i = 1; i <= 24; i++){
            if(regfile[i].used && regfile[i].target){
                if(regfile[i].target->name == arg->name && regfile[i].target->datalevel == arg->datalevel){
                    targetgre = regtypeToString(static_cast<RegType>(i));
                    lruRecentUse(i);
                    return targetgre;
                }
            }
        }
    }
    //reg未分配
    if(stmType == "Default"){
        for(int i = 1; i <= 24; i++){
            if(regfile[i].used == false){
                existEmpty = true;
                targetgre = regtypeToString(static_cast<RegType>(i));
                lruRecentUse(i);
                regfile[i].used = true;
                regfile[i].target = arg;
                break;
            }
        }
    }
    else if(stmType == "ReturnRes"){

    }
    if(!existEmpty){
        assert(targetgre == "");
        int lastuse = lruqueue[0];
        targetgre = regtypeToString(static_cast<RegType>(lastuse));
        lruRecentUse(lastuse);
    }

    return targetgre;
}

string FinalCodeGen::parseArgAddress(Arg *arg) {
    string off;
    if(arg->access == "indir"){

    }
    if(arg->form == "TempForm"){
        off = to_string(arg->dataoff);//*4
    }
    else if(arg->form == "AddrForm"){
        off = findOffsetByArg(arg);
    }
    else if(arg->form == "ValueForm") {
        assert(false);
    }
    return off;
}

string FinalCodeGen::findOffsetByArg(Arg *arg1) {
    int off = arg1->dataoff;
    int leveloff = 0;
    int m = callorder.size();
    bool isFindPro = false ;
    if(curgenPro->name == callorder[0][0]->procname){

    }
    else{
        for(int i = 0; i < m; i++){
            vector<ActiveRecord*> curchain = callorder[i];
            leveloff = 0;
            int chainnum = curchain.size();
            int tarcall = 0;
            for(int j = 0; j < chainnum ; j++){//调用顺序
                if(curchain[j]->procname == curgenPro->name){ //根据procname找sym是否在其中
                    isFindPro = true;
                    tarcall = j;
                    break;
                }
            }
            if(isFindPro) {
                bool isFindSym = false;
                for(int j = tarcall; j >= 0; j--)
                {
                    if(findFromScopeByProc_Name(curchain[j]->procname, arg1->name)){
                        isFindSym = true;
                    }
                    if(isFindSym) break;
                    leveloff += curchain[j]->totalsize;
                }
                if(isFindSym) break;
            }
        }
    }
    off += leveloff;
//    off *= 4; //因为一个int最后是4个单位
    return to_string(off);
}

Symbol *FinalCodeGen::findFromScopeByProc_Name(string procname, string symname) {
    SymbolTable* tar = nullptr;
    for(int i = 0; i < scope.size(); i++)
    {
        if(scope[i]->table[0]->name == procname){
            tar = scope[i];
            break;
        }
    }
    if(!tar) return nullptr;
    for(int i = 0; i < tar->table.size(); i++)
    {
        if(tar->table[i]->name == symname)
            return tar->table[i];
    }
    return nullptr;
}



string FinalCodeGen::mipsgenArith_R(string op, string rd, string rs, string rt) {
    string temp = op + ' ' + rd + ", " + rs + ", " + rt;
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

string FinalCodeGen::mipsgenArith_I(string op, string rt, string rs, string imme) {
    string temp = op + ' ' + rt + ", " + rs + ", " + imme;
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

string FinalCodeGen::mipsgenMem_I(string op, string rt, string rs, string off) {
    //lw,sw
    string temp = op + ' ' + rt + ", " + off + '(' + rs + ')';
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

string FinalCodeGen::mipsgenJump(string op, string label) {
    string temp = op + ' ' + label;
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

string FinalCodeGen::mipsgenBranchZ(string op, string rs,string rt, string label) {
    string temp = op + ' ' + rs + ", " + rt + ", " + label;
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

string FinalCodeGen::mipsgenLabel(string label) {
    string temp = label + ':';
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}
string FinalCodeGen::mipsgenNop() {
    string temp = "nop";
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

void FinalCodeGen::mipsgenStoreProcState() {
    mipsgenArith_I("addi", "$sp", "$sp", "-"+ to_string(curgenRecord->totalsize));//*4 addiu
    mipsgenMem_I("sw","$ra","$sp", to_string((curgenRecord->statestart+3*4))); // 存储$ra//*4
    mipsgenMem_I("sw","$fp","$sp", to_string((curgenRecord->statestart+1*4)));//存储$fp//*4
    mipsgenArith_R("addiu", "$fp", "$sp", "0");
}

string FinalCodeGen::mipsgenMemIndir(string op, string rt, string rs ) {
    string temp = op + " " + rt + " ,(" + rs + ")";
    mipsCode.push_back(temp);
    cout << temp << endl;
    return temp;
}

string RegManagement::regtypeToString(RegType x) {
    switch (x) {
        case $zero: return "$zero";
        case $at: return "$at";
        case $v0: return "$v0";
        case $v1: return "$v1";
        case $a0: return "$a0";
        case $a1: return "$a1";
        case $a2: return "$a2";
        case $a3: return "$a3";
        case $t0: return "$t0";
        case $t1: return "$t1";
        case $t2: return "$t2";
        case $t3: return "$t3";
        case $t4: return "$t4";
        case $t5: return "$t5";
        case $t6: return "$t6";
        case $t7: return "$t7";
        case $s0: return "$s0";
        case $s1: return "$s1";
        case $s2: return "$s2";
        case $s3: return "$s3";
        case $s4: return "$s4";
        case $s5: return "$s5";
        case $s6: return "$s6";
        case $s7: return "$s7";
        case $t8: return "$t8";
        case $t9: return "$t9";
        case $k0: return "$k0";
        case $k1: return "$k1";
        case $gp: return "$gp";
        case $sp: return "$sp";
        case $fp: return "$fp";
        case $ra: return "$ra";
        default: return "Unknown";
    }
}


