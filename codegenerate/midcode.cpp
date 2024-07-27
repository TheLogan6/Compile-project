#pragma GCC optimize("O0")
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <assert.h>
#include "global.h"
#include "midcode.h"
using namespace std;


#define LEXOPENFILEDIR "../Reference/test6addall.txt"
#define LEXOUTPUTFILEDIR "../Output/lexicaloutput.txt"

#define GRAOUTPUTFILEDIR "../Output/test6gra.txt"
#define SEMANOUTPUTFILEDIR "../Output/test6semantic.txt"
#define MIDCODEOUTPUTDIR "../Output/test6mid.txt"

//#define GRAMMARMAIN
//#define SEMANTICMAIN
//#define MIDCODEMAIN

#define MIDCODEDEBUG 1
#ifdef MIDCODEMAIN
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
    // freopen(GRAOUTPUTFILEDIR, "w", stdout); // 输出重定向
    cout << "Part2 : Grammar Analysis" << endl;  //奇怪的重定问题
    if (tokenList[pointer].wd.tok == ENDFILE) {
        cout << "grammar endfile !" << endl;
    }
    //tree->dfsPrintTree(t,0);
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

    int x= 0;
#endif
    fclose(fp);
}
#endif

void midCodeGenerate::grammarTreeScan() {
    tree->goRoot();
    TreeNode* p = tree->now;
    //programHead部分分析************
    tree->stepIn("ProgramHead");
    tree->stepIn("ID");
    prostack.push_back(tree->now->semantic);  //main函数声明
    codeAnalysisDeclare();
    codeAnalysisProgramBody();
    genCallChainMentry();

}

void midCodeGenerate::codeAnalysisDeclare() { //Declare主要针对procedure，以及函数嵌套
    tree->stepIn("DeclarePart");
    tree->stepIn("ProcDec");
    if(!tree->now->child.empty()){ //不能为空
        while(1)
        {
            if(tree->now->name == "ProcDecMore" && tree->now->child.empty()){
                break;
            }
            tree->stepIn("ProcName");
            tree->stepIn("ID");
            prostack.push_back(tree->now->semantic);
            tree->stepIn("ParamList");//过渡一下
            tree->stepIn("ProcDecPart");
            codeAnalysisDeclare(); //递归函数调用确实函数声明
            tree->stepIn("ProcBody");
            codeAnalysisProgramBody();
            tree->stepIn("ProcDecMore");
        }
    }
}

void midCodeGenerate::codeAnalysisProgramBody() {
    tree->stepIn("BEGIN");
    cout << "******Mid Code Program Begin*****" << endl;
    midgenProDec(); //当前的size是vari和param的大小
    midgenStmList();
    tree->stepIn("END");
    genRecordForMips();
    midgenProcEndCode();
}

void midCodeGenerate::midgenProDec() {
    codeindex ++;
    Symbol* curPro = prostack.back();
    curanalysispro = curPro;
    prostack.pop_back();
    labeltlb.addToList(curPro->name, codeindex);
    Arg* arg1 = new Arg("LabelForm",curPro->name, "","indir",curPro->level ,0,curPro);
    string proSize = (string)"SIZEOF" + "  " + to_string(curPro->size);//这个size是致函param和vari的
    Arg* arg2 = new Arg("ValueForm",proSize,proSize);
    string proLevel = to_string(curPro->level);
    Arg* arg3 = new Arg("ValueForm",proLevel, proLevel);
    string op;
    if(prostack.empty()){
        op = "MENTRY";  //main entry
    }
    else{
        op = "PENTRY"; // proc entry
    }
    FourArgExp* curfourargexp = new FourArgExp(op, codeindex, arg1,arg2,arg3);
    argExpList.push_back(curfourargexp);
    if(MIDCODEDEBUG) cout << curfourargexp->getfourArgExp() << endl;
}

void midCodeGenerate::midgenStmList() {
    tree->stepIn("Stm");
    while(1)
    {
        if(tree->now->name=="StmMore" && tree->now->child.empty()){
            break;
        }
        tree->stepIn("Stm");
        tree->preorderStep();
        string Stmlist_Choice = tree->now->name;
        if(Stmlist_Choice == "ConditionalStm"){
            midgenConditionalStm();
        }
        else if(Stmlist_Choice == "LoopStm"){
            midgenLoopStm();
        }
        else if(Stmlist_Choice == "InputStm"){
            midgenInputStm();
        }
        else if(Stmlist_Choice == "OutputStm"){
            midgenOutputStm();
        }
        else if(Stmlist_Choice == "ReturnStm"){
            assert(false); //虽然可以写，但我感觉没啥意义， 语法不支持 x = f() 这个类型
        }
            //详细处理
        else if(Stmlist_Choice == "ID"){
            Symbol* curvarsym = tree->now->semantic;
            tree->preorderStep();
            tree->stepIn("AssCall");
            tree->preorderStep();
            string Asscall_choice = tree->now->name;
            Arg* leftarg = nullptr;
            if(Asscall_choice == "AssignmentRest"){
                tree->stepIn("VariMore");
                //VariMore -> ε | [ Exp ] | . FieldVar
                if(tree->now->child.empty()){
                    leftarg = new Arg("AddrForm",curvarsym->name,"", curvarsym->access,curvarsym->level, curvarsym->offset,curvarsym);
                }
                tree->preorderStep();
                string varimore_choice = tree->now->name;
                if(varimore_choice  == "LMIDPAREN"){
                    leftarg = midgenArray(curvarsym);
                }
                else if(varimore_choice == "DOT"){
                    assert(false);
                }
                tree->stepIn("ASSIGN");
                Arg* rightarg = midgenExpression();
                midgenAssign(leftarg, rightarg);  //这里很好地解决了temp中间代码
            }
            else if(Asscall_choice == "CallStmRest"){
                //CallStmRest	::=	( ActParamList )   无参数调用也很重要
                tree->stepIn("ActParamList");
                vector<Arg*> temp_paramlist;
                while(1)
                {
                    if(tree->now->name == "ActParamList" && tree->now->child.empty()){
                        Symbol* curprosym = curvarsym;//函数无参调用
                        midgenProcedureCallCode(temp_paramlist, curprosym);
                        break;
                    }
                    if(tree->now->name == "ActParamMore" && tree->now->child.empty()){
                        break;
                    }
                    Arg* exparg = midgenExpression(); //参数变量
                    temp_paramlist.push_back(exparg);
                    tree->stepIn("ActParamMore");
                    Symbol* curprosym = curvarsym;
                    midgenProcedureCallCode(temp_paramlist, curprosym); //生成参数
                }

            }
            else assert(false);


        }
        tree->stepIn("StmMore");
    }
    return;
}
//IF RelExp THEN 	StmList ELSE StmList  FI
void midCodeGenerate::midgenConditionalStm() {
    tree->stepIn("IF");
    Arg* boolarg = midgenBoolExp();//判断执行Then
    tree->stepIn("THEN");
    labelgen ++ ;
    string elseLabel = "Lbl" + to_string(labelgen);
    Arg* elsearg = new Arg("LabelForm", elseLabel, elseLabel,"indir");
    midgenJumpCode("JUMP0", boolarg, elsearg); //否则进入else
    midgenStmList();
    tree->stepIn("ELSE");
    labelgen++;
    string outlabel = "Lbl" + to_string(labelgen);
    Arg* outarg = new Arg("LabelForm", outlabel, outlabel, "indir");
    midgenJumpCode("JUMP", outarg);
    midgenLabelCode("Label", elsearg); //else标签进入点
    midgenStmList();
    tree->stepIn("FI");
    midgenLabelCode("Label", outarg); //out of if
}

void midCodeGenerate::midgenLoopStm() {
    tree->stepIn("WHILE");
    labelgen++;
    string looplabel = "Lbl" + to_string(labelgen);
    Arg* looparg = new Arg("LabelForm", looplabel, looplabel, "indir");
    midgenLabelCode("Label", looparg); //Label start
    Arg* boolarg = midgenBoolExp();
    tree->stepIn("DO");
    labelgen++;
    string outlabel = "Lbl" + to_string(labelgen);
    Arg* outarg = new Arg("LabelForm", outlabel, outlabel, "indir");
    midgenJumpCode("JUMP0", boolarg, outarg);//jump
    midgenStmList();
    tree->stepIn("ENDWH");
    midgenJumpCode("JUMP", looparg);
    midgenJumpCode("Label", outarg); //跳出循环
}

//Exp		::=  Term   OtherTerm
Arg* midCodeGenerate::midgenExpression() {
    tree->stepIn("Exp");
    Arg* leftarg = midgenTerm();
    string op;
    tree->stepIn("OtherTerm");
    while(1)
    {
        if(tree->now->name == "OtherTerm" && tree->now->child.empty()){
            break;
        }
        //OtherTerm -> ε | AddOp Exp
        tree->stepIn("AddOp");
        tree->preorderStep();
        op = tree->now->tk->wd.str; //存的是+ -
//        cout << "op=" << op << endl;
        tree->stepIn("Exp");
        Arg* rightarg = midgenExpression();
        leftarg = midgenExpCode(op,leftarg,rightarg); //OtherTerm代表一定需要temp的
        tree->stepIn("OtherTerm");
    }
    return leftarg;
}

//Term -> Factor OtherFactor
Arg* midCodeGenerate::midgenTerm() {
    tree->stepIn("Term");
    Arg* leftarg = midgenFactor();
    string op;
    tree->stepIn("OtherFactor");
    while(1)
    {
        if(tree->now->name == "OtherFactor" && tree->now->child.empty()){
            break;
        }
        tree->stepIn("MultOp");
        tree->preorderStep();
        op = tree->now->tk->wd.str;
        tree->stepIn("Term");
        Arg* rightarg = midgenTerm();
        leftarg = midgenExpCode(op, leftarg, rightarg);  //同样OtherFactor代表一定有temp
        tree->stepIn("OtherFactor");
    }
    return leftarg;
}

Arg* midCodeGenerate::midgenFactor() {
    tree->stepIn("Factor");
    tree->preorderStep();
    TreeNode* p = tree->now;
    string factorchoice = p->name;
    if(factorchoice == "LPAREN"){
        return midgenExpression();
    }
    else if(factorchoice == "INTC"){
        string value = tree->now->tk->wd.str;
        Arg* leftarg = new Arg("ValueForm",value, value, "dir");
        return leftarg;
    }
    else if(factorchoice == "Variable"){
        return midgenVariable();
    }
    return nullptr;
}

Arg* midCodeGenerate::midgenVariable() {
    tree->stepIn("Variable");
    tree->stepIn("ID");
    TreeNode* p = tree->now;
    Symbol* curVarsym = tree->now->semantic;
    Arg* leftarg = nullptr;
    tree->stepIn("VariMore");
    if(tree->now->child.empty()){
        leftarg = new Arg("AddrForm",curVarsym->name, "", curVarsym->access, curVarsym->level, curVarsym->offset,curVarsym);
        return leftarg;
    }
    tree->preorderStep();
    string varimore_choice = tree->now->name;
    if(varimore_choice == "LMIDPAREN"){
        leftarg = midgenArray(curVarsym);
        return leftarg;
    }
    else if(varimore_choice == "DOT"){
        cout << "暂时放弃" << endl;
        assert(false);
    }
}

Arg* midCodeGenerate::midgenExpCode(string op, Arg *left, Arg *right) {//产生TempForm,用于i-low
    codeindex ++ ;
    tempvargen ++;
    op = symbolToWord(op);
    string tmpVarName = "temp" + to_string(tempvargen);
    string tmpacc = "dir";
    if(op == "AADD") tmpacc = "indir";
    Arg* tmparg = new Arg("TempForm", tmpVarName, tmpVarName, tmpacc, curanalysispro->level+1, curanalysispro->size);
    curProAddTempVari(tmparg);
    FourArgExp* curfourexp = new FourArgExp(op, codeindex, left, right, tmparg);
    argExpList.push_back(curfourexp);
    if(MIDCODEDEBUG) cout << curfourexp->getfourArgExp() << endl;
    return tmparg;
}

void midCodeGenerate::midgenAssign(Arg *left, Arg *right) {
    codeindex ++;
    FourArgExp* curargexp = new FourArgExp("ASSIGN", codeindex, left, right);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
    return ;
}

string midCodeGenerate::symbolToWord(string op) {
    if (op == "-") {
        return "SUB";
    } else if (op == "+") {
        return "ADD";
    } else if (op == "*") {
        return "MULT";
    } else if (op == "/") {
        return "DIV";
    } else if (op == "<") {
        return "LTC";
    } else if (op == "=") {
        return "EQC";
    } else if (op == "AADD") {
        return "AADD";
    } else {
        assert(false);
        return "ERROR";
    }
}

Arg *midCodeGenerate::midgenBoolExp() {
    tree->stepIn("RelExp");
    Arg* leftarg = midgenExpression();
    tree->stepIn("CmpOp");
    tree->preorderStep();
    string op = tree->now->tk->wd.str;
    Arg* rightarg = midgenExpression();
    Arg* temparg = midgenBoolCode(op,leftarg, rightarg);
    return temparg;

}

Arg *midCodeGenerate::midgenBoolCode(string op, Arg *left, Arg *right) { // ReExp同Exp，产生TempForm
    codeindex++;
    tempvargen ++ ;
    op = symbolToWord(op);
    string tempvarname = "temp" + to_string(tempvargen);
    Arg* temparg = new Arg("TempForm", tempvarname, tempvarname, "dir", curanalysispro->level+1, curanalysispro->size);
    curProAddTempVari(temparg);
    FourArgExp* curargexp = new FourArgExp(op, codeindex, left, right, temparg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
    return temparg;
}

void midCodeGenerate::midgenJumpCode(string op, Arg *boolarg, Arg *elsearg) {
    codeindex++;
    FourArgExp* curargexp = new FourArgExp(op, codeindex, boolarg, elsearg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
    return;
}
//重载
void midCodeGenerate::midgenJumpCode(string op, Arg *boolarg) {
    codeindex++;
    FourArgExp* curargexp = new FourArgExp(op, codeindex, boolarg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
    return ;
}

void midCodeGenerate::midgenLabelCode(string op, Arg *labelarg) {
    codeindex++;
    labeltlb.addToList(labelarg->name, codeindex);
    FourArgExp* curargexp = new FourArgExp(op, codeindex, labelarg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
    return ;
}

void midCodeGenerate::midgenInputStm() {
    tree->stepIn("InputStm");
    tree->stepIn("READ");
    tree->stepIn("ID");
    codeindex ++ ;
    Symbol* curvarsym = tree->now->semantic;
    Arg* curarg = new Arg("AddrForm", curvarsym->name, curvarsym->name, curvarsym->access, curvarsym->level, curvarsym->offset,curvarsym);
    FourArgExp* curargexp = new FourArgExp("READC",codeindex,curarg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
}

void midCodeGenerate::midgenOutputStm() {
    tree->stepIn("OutputStm");
    tree->stepIn("WRITE");
    codeindex++;
    Arg* leftarg = midgenExpression();
    FourArgExp* curargexp = new FourArgExp("WRITEC",codeindex,leftarg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
}

void midCodeGenerate::midgenReturnStm() {
    tree->stepIn("ReturnStm");
    tree->stepIn("RETURN");
    midgenExpression();
    midgenReturnCode("RETURNC");
}
void midCodeGenerate::midgenProcedureCallCode(vector<Arg *> param, Symbol *proc) {
    int paramnum = 0;
    int m = param.size();
    for(int i = 0; i < m ;i++){
        Symbol* sem_param = proc->param->table[i];
        Arg* arg_param = param[i];
        string op;
        if(sem_param->access == "indir"){
            op = "VARACT";
        }
        else{
            op = "VALACT";
        }
        midgenParamCode(op, arg_param, sem_param->offset, i); //参数生成
    }
    Arg* labelarg = new Arg("LabelForm",proc->name, proc->name, "indir");
    FourArgExp* curargexp = new FourArgExp("CALL", codeindex, labelarg); //CALL代码
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
}

Arg *midCodeGenerate::midgenArray(Symbol* arrayid) {
    Arg* leftarg = midgenExpression();
    int arraylow = arrayid->tp->at->low;
    int arraysize = arrayid->tp->at->element->size;
    Arg* rightarg = new Arg("ValueForm", to_string(arraylow), to_string(arraylow),"dir"); //ValueForm
    leftarg = midgenExpCode("-", leftarg, rightarg);//a[x] 生成x-1
//    free(rightarg);
    rightarg = new Arg("ValueForm", to_string(arraysize), to_string(arraysize),"dir");
    rightarg = midgenExpCode("*", leftarg, rightarg);
    Arg* vararg = new Arg("AddrForm", arrayid->name, arrayid->name, arrayid->access, arrayid->level, arrayid->offset,arrayid);
    leftarg = midgenExpCode("AADD", vararg, rightarg);
    return leftarg;
}

void midCodeGenerate::midgenParamCode(string op, Arg *arg, int off, int paramindex) {
    codeindex++;
    Arg* offarg = new Arg("ValueForm", to_string(off), to_string(off));
    Arg* paramindexarg = new Arg("ValueForm", to_string(paramindex), to_string(paramindex));
    FourArgExp* curargexp = new FourArgExp(op, codeindex, arg, offarg, paramindexarg);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
}

void midCodeGenerate::midgenProcEndCode() {
    codeindex++;
    FourArgExp* curargexp = new FourArgExp("ENDPROC", codeindex);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
}



void midCodeGenerate::midgenReturnCode(string op) {
    codeindex++;
    FourArgExp* curargexp = new FourArgExp(op, codeindex);
    argExpList.push_back(curargexp);
    if(MIDCODEDEBUG) cout << curargexp->getfourArgExp() << endl;
    return ;
}

void midCodeGenerate::curProAddTempVari(Arg* tmparg) {
    curanalysispro->size += 4 ;
    curanalysispro->tempvarisize += 4 ;
    return ;
}

void midCodeGenerate::genRecordForMips() {
    Symbol* curpro = curanalysispro;
    int varisize = curpro->size - curpro->paramsize - curpro->tempvarisize; //这是temp的起点 修改
    ActiveRecord* curactrecord = new ActiveRecord(curpro->name, curpro->size+16, 0, varisize, \
     varisize + curpro->tempvarisize , curpro->size, curpro->level+1 );
    actrecord.push_back(curactrecord);
    return;
}

ActiveRecord *midCodeGenerate::findActRedByName(string tar) {
    int m = actrecord.size();
    for(int i = 0; i < m; i++){
        if(actrecord[i]->procname == tar)
            return actrecord[i];
    }
    return nullptr;
}

int midCodeGenerate::findArgListIndexByName(string tar) {
    int n = argExpList.size();
    for(int i = 0; i < n; i++)
    {
        if(argExpList[i]->codekind == "PENTRY"){
            if(argExpList[i]->arg1->name == tar)
                return i;
        }
    }
    return -1;
}

void midCodeGenerate::genCallChainForMips(vector<ActiveRecord*> curact, int i) {
    int t = i+1;
    for(int j = t; j < argExpList.size(); j++)
    {
        if(argExpList[j]->codekind == "ENDPROC"){
            callorder.push_back(curact);
            return;
        }
        else if(argExpList[j]->codekind == "CALL"){
            string callprocname = argExpList[j]->arg1->name;
            curact.push_back(findActRedByName(callprocname));
            int proindex = findArgListIndexByName(callprocname);
            genCallChainForMips(curact, proindex);
            curact.pop_back();
        }
    }
    return ;
}

void midCodeGenerate::genCallChainMentry() {
    //下面顺便生成调用链
    int n = argExpList.size();
    int mainp  = 0;
    vector<ActiveRecord*> curcall;
    for(int i = 0; i < n; i++)
    {
        if(argExpList[i]->codekind == "MENTRY"){
            mainp = i;
            curcall.push_back(findActRedByName(argExpList[i]->arg1->name));
            genCallChainForMips(curcall, i);
            break;
        }
    }
}

void midCodeGenerate::beautPrintExp() {
    int n = argExpList.size();
    cout << "*****整理四元式*****" << endl;
    for(int i = 0; i < n; i++)
    {
        FourArgExp* curarg = argExpList[i];
        curarg->beautPrint();
    }
}


