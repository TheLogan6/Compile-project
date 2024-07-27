#include "global.h"
#include "lexical.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <assert.h>
#include "grammar.h"
#include "semantic.h"
using namespace std;

extern Token tokenList[1010];
extern int pointer; //treepointer
extern GrammarTree* tree;
#define SEMANTICDEBUG 1
#define ERROREXIT 0 //理论上是需要打开的，一遇错误直接退出版本
//#define GRAMMARMAIN
//#define SEMANTICMAIN

#ifdef SEMANTICMAIN
#define LEXOPENFILEDIR "../Reference/test4.txt"
#define LEXOUTPUTFILEDIR "../Output/lexicaloutput.txt"

#define GRAOUTPUTFILEDIR "../Output/test4gra.txt"
#define SEMANOUTPUTFILEDIR "../Output/test4semantic.txt"
int main(){
    initReservedWord(); //初始化保留字
    FILE *fp;
    std::ofstream outputFile(SEMANOUTPUTFILEDIR, std::ios::out | std::ios::binary);
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
    analyzer.getSemanticList(analyzer.tree->root);
    int x = 1;
    fclose(fp);
#endif
}
#endif


void Analyzer::semanticAnalyze() {
    //tree是现在的核心
    initSemantic();
    grammarToSynax();
    cout << "*******主程序运行语义检测开始*******" << endl;
    synaxProgramAnalyze();
    cout << "*******主程序运行语义检测完毕*******" << endl;
    cout << endl << "*******语义类型检测总结*******" << endl;
    printmessage();
    calcuProcSize();
}

void Analyzer::initSemantic() {
    levelCurrent += 1;
    offsetCurrent.push_back(0); //offset的处理
}

void Analyzer::grammarToSynax() {
    cout << "***************语义分析开始***************"  << endl;
    tree->stepIn("ProgramName");
    tree->stepIn("ID");
    string proc_name = tree->now->tk->wd.str;
    Symbol* procsym = new Symbol(proc_name, ProcDecK, nullptr, tree->now->tk);
    procsym->procSymbolCons(nullptr, levelCurrent, offsetCurrent.back());
    procsym->param = nullptr;
    tree->now->semantic = procsym;
    tbSym->addSymbol(procsym);
    procsym->printSymbol();

    levelCurrent += 1;
    offsetCurrent.push_back(0);

    cout << "********声明部分********"  << endl;
    synaxDeclareAll();
}

void Analyzer::synaxDeclareAll() {
    tree->stepIn("DeclarePart");
    synaxTypeDec();
    synaxVarDec();
    synaxProDec();
}

void Analyzer::synaxTypeDec() {
    tree->stepIn("TypeDec");
    TreeNode* p = tree->now;
    if(p->child.empty())         // TypeDec -> kong | TypeDecList
        return;
    tree->stepIn("TypeDecList"); // 这是核心
    p = tree->now;
    while(1)
    {
        if(tree->now->name == "TypeDecMore" && tree->now->child.empty())
            break;
        tree->stepIn("TypeId");
        tree->stepIn("ID");  //自定义ID，typedef类型
        p = tree->now;
        string type_id = tree->now->tk->wd.str;
        type* id_type = getIdType();
        Symbol* sym = new Symbol(type_id, TypeK, id_type, p->tk);
        tree->now->semantic = sym;
        sym->printSymbol();

        if(id_type != nullptr){  //ID类型的type，需要进行符号检测
            sym->tp = id_type;
            if(tbSym->findByName(sym->name)){
                ReDefine e(sym->name, sym->tk->line);
                cout << e.what() << endl;
                if(ERROREXIT) exit(1);
            }
            else{
                tbSym->addSymbol(sym);
            }
        }
        tree->stepIn("TypeDecMore");
    }

}
void Analyzer::synaxVarDec() {
    tree->stepIn("VarDec");//VarDec -> ε | VarDeclaration
    TreeNode* p = tree->now;
    if(p->child.empty()){
        return ;
    }
    tree->stepIn("VarDecList");
    while(1)
    {
        if(tree->now->name == "VarDecMore" && tree->now->child.empty()){
            for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
            cout << "*******变量声明结束*******" << endl;
            break;
        }
        type* id_type = getIdType();
        tree->stepIn("VarIdList");
        while(1)
        {
            if(tree->now->name == "VarIdMore" && tree->now->child.empty()){
                break;
            }
            tree->stepIn("ID");
            TreeNode* tt = tree->now;
            Symbol* sym =  new Symbol(tt->tk->wd.str, VarK, id_type, tt->tk);
            sym->varSymbolCons("dir",levelCurrent, offsetCurrent.back());
            offsetCurrent.back() += sym->getSymbolSize();
            if(tbSym->findByName(tree->now->tk->wd.str)){
                ReDefine e(sym->name, sym->tk->line);
                cout << e.what() << endl;
                if(ERROREXIT) exit(1);
            }
            else{
                tt->semantic = sym;
                tbSym->addSymbol(sym);
                sym->printSymbol();
                tree->stepIn("VarIdMore"); //同一类型多个变量
            }

        }
        tree->stepIn("VarDecMore");
    }
}
void Analyzer::synaxProDec() {
    SymbolTable* tbSymCur = tbSym; // 暂存
    tree->stepIn("ProcDec"); //ProcDec -> ε | ProcDeclaration
    if(tree->now->child.empty()){
        return ;
    }
    tree->preorderStep();

    while(1)
    {
        if(tree->now->name == "ProcDecMore" && tree->now->child.empty()){
            break;
        }
        tree->stepIn("ProcName");
        tree->stepIn("ID");

        string proc_name = tree->now->tk->wd.str;
        Symbol* procsym = new Symbol(proc_name,ProcDecK, nullptr,tree->now->tk);
        procsym->procSymbolCons(new SymbolTable(), levelCurrent, offsetCurrent.back());

        tree->now->semantic = procsym; //再次丰富语法树

        //参数Table的建立
        SymbolTable* paramsymTable = new SymbolTable();
        paramsymTable->addSymbol(procsym);

        if(tbSym->findByName(proc_name)){// proc标识符重定义了
            ReDefine e(procsym->name, procsym->tk->line);
            cout << e.what() << endl;
            if(ERROREXIT) exit(1);;
        }
        else{
            tbSym->addSymbol(procsym);
            for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
            procsym->printSymbol();
        }
        scope.push_back(paramsymTable);  //加入新的过程，过程的开头是proc
        this->tbSym = scope.back();
        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        cout << "***进入自定义" << procsym->name << "函数分析***" << endl;
        tree->stepIn("ParamList");
        tree->preorderStep();
        //这里是ParamList，参数范围
        levelCurrent += 1;
        offsetCurrent.push_back(0);
        //开始归零计算
        //ParamList -> ε | ParamDecList
        if(!tree->now->child.empty())
        {
            while(1)
            {
                if(tree->now->name == "ParamMore" && tree->now->child.empty()){
                    break;
                }
                tree->stepIn("Param");
                //Param -> TypeName FormList | VAR TypeName FormList
                //这里基本没有VAR的说法？ 有小bug，但是感觉可以忽略
                type* id_type = getIdType();
                if(id_type == nullptr){
                    assert(false); // 不可能false理论上
                }
                tree->stepIn("FormList");
                while(1)
                {
                    if(tree->now->name == "FidMore" && tree->now->child.empty()){
                        break;
                    }
                    //FormList -> ID FidMore
                    tree->stepIn("ID");
                    Symbol* paramsym = new Symbol(tree->now->tk->wd.str,VarK,id_type, tree->now->tk);
                    paramsym->varSymbolCons("dir", levelCurrent, offsetCurrent.back());
                    procsym->param->addSymbol(paramsym);

                    tree->now->semantic = paramsym;
                    offsetCurrent.back() += paramsym->getSymbolSize();
                    if(tbSym->findByName(paramsym->name)){
                        ReDefine e(paramsym->name, paramsym->tk->line);
                        cout << e.what() << endl;
                        if(ERROREXIT) exit(1);;
                    }
                    else{
                        tbSym->addSymbol(paramsym); // 大表也要加入
                        paramsym->printSymbol();
                    }
                    //FidMore -> ε | , FormList
                    tree->stepIn("FidMore");
                }
                tree->stepIn("ParamMore");
            }
        }
        tree->stepIn("ProcDecPart");
        this->synaxDeclareAll();
        tree->stepIn("ProcBody");
        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        cout << "***检查自定义" << procsym->name << "函数程序体***" << endl;
        synaxProgramAnalyze();

        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        cout << "***自定义" << procsym->name << "函数检查完毕***" << endl;
        tbSym = tbSymCur; // 恢复现状

        tree->stepIn("ProcDecMore");//More Proc声明
        levelCurrent -= 1;
        offsetCurrent.pop_back();
    }
}


//这是全部的分析类型Inner属性时的操作
type* Analyzer::getIdType() { //针对的就是TypeName的解析
    tree->stepIn("TypeName");
    //TypeName -> BaseType | StructureType | ID
    string base_stru = tree->now->child[0]->name;
    tree->stepIn(base_stru);
    if(base_stru == "BaseType")
    {
       return getBaseType();
    }
    else if(base_stru == "StructureType")
    {
        string struc_typename = tree->now->child[0]->name;
        tree->stepIn(struc_typename);
        if(struc_typename == "ArrayType"){
             return getArrayType();
        }
        //RecType -> RECORD FieldDecList END
        else if(struc_typename == "RecType"){//在变量定义阶段
            type* tp = new type();
            int tmpsize = 0, tmpoff = 0;
            SymbolTable* filedList = new SymbolTable();
            tree->stepIn("RECORD");
            //FieldDecList -> BaseType IdList ; FieldDecMore | ArrayType IdList ; FieldDecMore
            while(1)
            {
                if(tree->now->name == "FieldDecMore" && tree->now->child.empty()){
                    break;
                }
                tree->stepIn("FieldDecList");
                string filetypekind = tree->now->child[0]->name;//getNodeKind
                type* filedinnertype = nullptr;
                if(filetypekind == "BaseType"){
                    filedinnertype = getBaseType();
                }
                else if(filetypekind == "ArrayType"){
                    filedinnertype = getArrayType();
                }
                while(1)
                {
                    if(tree->now->name == "IDMore" && tree->now->child.empty()){
                        break;
                    }
                    tree->stepIn("IDList");
                    tree->stepIn("ID");
                    TreeNode* tt = tree->now;
                    Symbol* filedsym = new Symbol(tree->now->tk->wd.str, VarK, filedinnertype, tree->now->tk);
                    filedsym->varSymbolCons("dir",levelCurrent, tmpoff);
                    tmpoff += filedsym->tp->getSize();
                    tmpsize += filedsym->tp->getSize();
                    int filedsavesize = filedList->table.size();
                    bool recordinnerRe = false;
                    for(int i = 0; i < filedsavesize; i++){
                        if(filedsym->name == filedList->table[i]->name){
                            recordinnerRe = true;
                            break;
                        }
                    }
                    if(recordinnerRe){
                        ReDefine e(tt->tk->wd.str, tt->tk->line);
                        cout << e.what() << endl;
                        if(ERROREXIT) exit(1);
                    }
                    else{
                        filedList->addSymbol(filedsym);
                    }
                    tree->stepIn("IDMore");
                }
                tree->stepIn("FieldDecMore");
            }
            tree->stepIn("END");
            recordtype* rectp = new recordtype(tmpsize, filedList);
            tp->rt = rectp;
            return tp;
        }
    }
    else if(base_stru == "ID")
    {
        tree->stepIn("ID");
        TreeNode* p = tree->now;
        string define_typename = tree->now->tk->wd.str;
        if(tbSym->findByName(define_typename)){
            Symbol* typesym = tbSym->findByName(define_typename);
            return typesym->tp;
        }
        else{
            UndefinedSymbol e(define_typename, p->tk->line);
            cout << e.what() << endl;
            return nullptr;
        }
    }
    else{
        assert(false);
    }


}

type* Analyzer::getBaseType() {
    tree->stepIn("BaseType");
    tree->preorderStep(); //这条已经是这样了
    string type_name = tree->now->getNodeKind();  //integer 或者 char
//    cout << "type:" << type_name << endl;
    type* t = new type();
    t->bt = new basetype(4,type_name);
    return t;
}

//ArrayType -> ARRAY [ Low .. Top ] OF BaseType
type *Analyzer::getArrayType() {

    type* array_type = new type();
    tree->stepIn("ArrayType");

    tree->stepIn("Low");
    tree->stepIn("INTC");
    TreeNode* p = tree->now;
    string array_low = tree->now->tk->wd.str;
    tree->stepIn("Top");
    tree->stepIn("INTC");
    string array_top = tree->now->tk->wd.str;
    tree->stepIn("BaseType");
    tree->preorderStep();
    int array_low2 = stoi(array_low);
    int array_top2 = stoi(array_top);
    basetype* array_ele_basetype = new basetype(4, enumToString(tree->now->tk->wd.tok));
    arraytype* at = new arraytype(array_low2, array_top2, array_ele_basetype);
    array_type->at = at;

    if(array_low2 < 0 || array_low2 >= array_top2){
        ArrayDefineError e(tree->now->tk->line);
        cout << e.what() << endl;
    }
    else{
        string CorrectMes = "Array define correctly in line" + to_string(p->tk->line);
        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        if(SEMANTICDEBUG) cout << CorrectMes << endl;
        runMessgae.push_back(CorrectMes);
    }
    return array_type;
}


void Analyzer::synaxProgramAnalyze() {
    tree->stepIn("BEGIN");
    for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
//    cout << "程序体 Begin" << endl;
    //ProgramBody -> BEGIN StmList END
    synaxStmList();
    tree->stepIn("END");

}
void Analyzer::synaxStmList() {
    tree->stepIn("StmList");
    //StmList -> Stm StmMore
    TreeNode* p = tree->now;
    while(1)
    {
        if(tree->now->name == "StmMore" && tree->now->child.empty()){
            break;
        }
        tree->stepIn("Stm");
        tree->preorderStep();
        p = tree->now;
        //Stm -> ConditionalStm | LoopStm | InputStm |OutputStm | ReturnStm | ID AssCall
        string curstmtype = tree->now->name;
        if(curstmtype == "ConditionalStm"){
            synaxConditionalStm();
        }
        else if(curstmtype == "LoopStm"){
            synaxLoopStm();
        }
        else if(curstmtype == "InputStm"){
            synaxInputStm();
        }
        else if(curstmtype == "OutputStm"){
            synaxOutputStm();
        }
        else if(curstmtype == "ReturnStm"){
            synaxReturnStm();
        }
        //详细处理
        else if(curstmtype == "ID"){
            TreeNode* pp = tree->now;
            string id_name = tree->now->tk->wd.str;
            tree->stepIn("AssCall");//Assmeble call
//            AssCall -> AssignmentRest | CallStmRest
            tree->preorderStep();//ID 或者 (
            string asscall_choice = tree->now->name;
            bool varNotInTable = true;
            type* assignVarType = nullptr;
            Symbol* assignVarSymbol = nullptr;
            //先测试ID
//            int scopenum = levelCurrent;
            int scopenum = scope.size();
            bool findNearSameLevel = false;
            for(int i = scopenum-1; i >= 0; i--)
            {
                SymbolTable* tempsymtbl = scope[i];
                if(tempsymtbl->table[0]->level+1 > levelCurrent) continue;
                if(tempsymtbl->table[0]->level+1 == levelCurrent){
                    if(!findNearSameLevel)
                        findNearSameLevel = true;
                    else continue;
                }
                if(tempsymtbl->findByName(id_name)){
                    assignVarSymbol = tempsymtbl->findByName(id_name);
                    if(assignVarSymbol->decKind == TypeK){
                        LAssignInvalid e(assignVarSymbol->name, pp->tk->line,"TypeK");
                        if(ERROREXIT) exit(1);
                    }
                    varNotInTable = false;
                    pp->semantic = assignVarSymbol;
                    break;
                }
            }
            if(varNotInTable){// 没有在scope中找到
                UndefinedSymbol e(id_name, pp->tk->line);
                cout << e.what() << endl;
                if(ERROREXIT) exit(1);;
            }
//            AssignmentRest -> VariMore := Exp, CallStmRest -> ( ActParamList )
            if(asscall_choice == "AssignmentRest"){
                assignVarType = nullptr;
              //AssignmentRest -> VariMore := Exp
                tree->stepIn("VariMore");
                //VariMore -> ε | [ Exp ] | . FieldVar
                if(tree->now->child.empty()){//VariMore -> ε
                    if(!varNotInTable){
                        assignVarType = assignVarSymbol->tp;
                    }
                }
                tree->preorderStep();
                string varimore_choice = tree->now->name;
                if(varimore_choice == "LMIDPAREN"){ // VariMore -> [ Exp ]
                    //针对数组 [x]
                    Symbol* array_index = synaxExpression();
                    string arr_index_type = array_index->getType();
                    string arr_index_value = array_index->tk->wd.str;
                    int arr_index_lineno = array_index->tk->line;
                    if(arr_index_type == "INTEGER" || arr_index_type == "INTC"){
                        if(arr_index_type == "INTC"){
                            int indexval = stoi(arr_index_value);
                            int definelow = assignVarSymbol->tp->at->low;
                            int definehigh = assignVarSymbol->tp->at->top;
                            if(indexval >= definelow && indexval <= definehigh){
                                string CorrectMes = "Array access correctly in line " + to_string(arr_index_lineno);
                                for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
                                cout << CorrectMes << endl;
                                runMessgae.push_back(CorrectMes);
                            }
                            else{
                                ArrayIndexOutBound e(pp->name, arr_index_lineno, definelow, definehigh);
                                cout << e.what() << endl;
                            }
                        }
                        else{
                            string CorrectMes = "Array access correctly in line " + to_string(arr_index_lineno);
                            for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
                            cout << CorrectMes << endl;
                            runMessgae.push_back(CorrectMes);
                        }
                    }
                    else{
                        ArrayIndexMisMatch e(pp->name, arr_index_lineno);
                        cout << e.what() << endl;
                    }
                    assignVarType = assignVarSymbol->tp;
                }
                else if(varimore_choice == "DOT"){
                    //针对结构体
                    tree->stepIn("FieldVar");
                    tree->stepIn("ID");
                    TreeNode* tt = tree->now;
                    //FieldVar -> ID FieldVarMore
                    //FieldVarMore -> ε | [ Exp ]
                    string filedName = tree->now->tk->wd.str;
                    if(assignVarSymbol->getType() != "RecordType"){
                        exit(1);
                    }
                    SymbolTable* filedTable = assignVarSymbol->tp->rt->filedList;
                    int filednum = filedTable->table.size();
                    bool findFiledVar = false;
                    Symbol* filednow = nullptr;
                    for(int i = 0; i < filednum; i++){
                        if(filedTable->table[i]->name == filedName){
                            filednow = filedTable->table[i];
                            tree->now->semantic = filednow;
                            findFiledVar = true;
                        }
                    }
                    if(!findFiledVar){
                        UndefinedSymbol e(filedName, tt->tk->line);
                        cout << e.what() << endl;
                        if(ERROREXIT) exit(1);
                    }
                    else{
                        string CorrectMes = "Record access correctly in line " + to_string(tt->tk->line);
                        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
                        cout << CorrectMes << endl;
                        runMessgae.push_back(CorrectMes);
                    }
                    tree->stepIn("FieldVarMore");
                    if(tree->now->child.empty()){
                        assignVarType = filednow->tp;
                    }
                    else if(tree->now->tk->wd.tok == LMIDPAREN){
                        Symbol* filedarr = synaxExpression();
                        string indexType = filedarr->name;
                        string indexValue = filedarr->tk->wd.str;
                        assert(false);
                    }
                }
                tree->stepIn("ASSIGN"); //:= Exp
                Symbol* right_expsym = synaxExpression();
                string right_type = "";
                if(right_expsym){
                    right_type = right_expsym->getType();
                    string left_type = assignVarType->getType();
                    if(!assign_typecheck(assignVarType->getType(), right_type)){
                        AssignMisMatch e(assignVarType->getType(), right_type, pp->tk->line);
                        cout << e.what() << endl;
                    }
                    else{
                        string CorrectMes = "AssignStm run correctly in line " + to_string(pp->tk->line);
                        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
                        cout << CorrectMes << endl;
                        runMessgae.push_back(CorrectMes);
                    }
                }
            }
            else if(asscall_choice == "CallStmRest"){
                tree->stepIn("ActParamList");//实参
                vector<Symbol*> act_paramlist;
                //需要定义一个vector存储param
                //ActParamList -> ε | Exp ActParamMore
                while(1)
                {
                    if(tree->now->name == "ActParamList" && tree->now->child.empty()){
                        break;
                    }
                    if(tree->now->name == "ActParamMore" && tree->now->child.empty()){
                        break;
                    }
                    Symbol* exp_sym = synaxExpression();
                    string exp_type = exp_sym->name;
                    act_paramlist.push_back(exp_sym);
                    tree->stepIn("ActParamMore");
                }
                //参数获取完毕
                int actual_arglen = act_paramlist.size();
                Symbol* proccallsym = assignVarSymbol;
                if(proccallsym->decKind != ProcDecK){  //调用报错
                    ProcedureCallError e(proccallsym->name, pp->tk->line);
                    cout << e.what() <<endl;
                }
                else
                {
                    SymbolTable* define_paralist = proccallsym->param;
                    int define_paranum = define_paralist->table.size();
                    if(define_paranum != actual_arglen){
                        ArgumentNumMismatch e(proccallsym->name, pp->tk->line, actual_arglen);
                        cout << e.what() << endl;
                    }
                    else{
                        for(int i = 0; i < define_paranum; i++)
                        {
                            string define_paramtype = define_paralist->table[i]->getType();
                            string actual_argtype = act_paramlist[i]->getType();
                            if(!assign_typecheck(define_paramtype, actual_argtype)){
                                ProcedureParamMisMatch e(actual_argtype, define_paramtype, act_paramlist[i]->tk->line);
                                cout << e.what() << endl;
                            }
                        }
                        string CorrectMes = "CallStm run correctly in line " + to_string(pp->tk->line);
                        cout <<CorrectMes << endl;
                        runMessgae.push_back(CorrectMes);
                    }
                }
            }
            else   assert(false); //除了AssignmentRest和CallStmRest
        }
        else{
            assert(false);
        }
        tree->stepIn("StmMore");
    }
}

void Analyzer::synaxInputStm() {
    //InputStm -> READ ( Invar )
    tree->stepIn("InputStm");
    tree->stepIn("Invar");
    tree->stepIn("ID");
    TreeNode* p = tree->now;
    string id_name = tree->now->tk->wd.str;
    if(tbSym->findByName(id_name)){ //找到
        tree->now->semantic = tbSym->findByName(id_name);
        string CorrectMes = "InputStm run correctly in line " + to_string(p->tk->line);
        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        cout << CorrectMes << endl;
        runMessgae.push_back(CorrectMes);
    }
    else{ //未定义错误
        UndefinedSymbol e(id_name, p->tk->line);
        cout << e.what() << endl;
        if(ERROREXIT) exit(1);;
    }
}

void Analyzer::synaxOutputStm() {
    //OutputStm -> WRITE ( Exp )
    tree->stepIn("OutputStm");
    tree->stepIn("WRITE");
    Symbol* tmp = synaxExpression();
    if(tmp){
        string CorrectMes = "OutputStm run correctly in line " + to_string(tmp->tk->line);
        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        cout << CorrectMes << endl;
        runMessgae.push_back(CorrectMes);
    }
}

//Exp -> Term OtherTerm
//Term -> Factor OtherFactor 运算符
//OtherTerm -> ε | AddOp Exp
Symbol* Analyzer::synaxExpression() {
    tree->stepIn("Exp");
    Symbol* left_termsym = synaxTerm();
    string leftsym_type = left_termsym->getType();
    string leftsym_val = left_termsym->tk->wd.str;
    tree->stepIn("OtherTerm");//加法一般项
    while(1)
    {
        if(tree->now->name == "OtherTerm" && tree->now->child.empty()){
            break;
        }
        tree->stepIn("Exp");
        Symbol* right_expsym = synaxExpression();
        string rightsym_type = right_expsym->getType();
        string rightsym_val = right_expsym->tk->wd.str;
        if(!assign_typecheck(leftsym_type, rightsym_type)){
            AssignMisMatch e(leftsym_type, rightsym_type, left_termsym->tk->line);
            cout << e.what() <<endl;
            if(ERROREXIT) exit(1);;
        }
        tree->stepIn("OtherTerm");
    }
    return left_termsym;
}
//Term -> Factor OtherFactor 运算符
//OtherFactor  ->  ε    |  MultOp  Term
Symbol* Analyzer::synaxTerm() {
    tree->stepIn("Term");
    Symbol* left_factor_sym = synaxFactor();
    string leftsym_type = left_factor_sym->getType();
    string leftsym_val = left_factor_sym->tk->wd.str;
    tree->stepIn("OtherFactor");//乘法因子
    bool termError = false;
    while(1)
    {
        if(tree->now->name == "OtherFactor" && tree->now->child.empty()){
            break;
        }
        tree->stepIn("Term");
        Symbol* right_factor_sym = synaxFactor();
        string rightsym_type = right_factor_sym->getType();
        string rightsym_val = right_factor_sym->tk->wd.str;
        //比较左值和右值的类型
        if(!assign_typecheck(leftsym_type, rightsym_type)){
            AssignMisMatch e(leftsym_type, rightsym_type, left_factor_sym->tk->line);
            cout << e.what() << endl;
            termError = true;
            if(ERROREXIT) exit(1);;
        }
        tree->stepIn("OtherFactor");
    }

    if(termError){
        return nullptr;
    }
    return left_factor_sym;
} // Term返回的是 type 和 val 同样可能是 type* 或者 string

//Factor -> ( Exp ) | INTC | Variable
Symbol* Analyzer::synaxFactor() { // 返回的是
    tree->stepIn("Factor");
    tree->preorderStep();
    TreeNode* p = tree->now;
    string factorchoice = p->name;
    if(factorchoice == "LPAREN"){
        return synaxExpression();
    }
    else if(factorchoice == "INTC"){
        Symbol* intc_sym = new Symbol("INTC", ExpK, nullptr, p->tk);
        //主要使用tpye和val，即"INTC", p->tk->wd.str
        return intc_sym;
    }
    else if(factorchoice == "Variable"){
        return synaxVariable();
    }
    else assert(false);
}

//Variable -> ID VariMore  变量处理
Symbol* Analyzer::synaxVariable() { //这个return tpye* 和 nodeval 或者 nodekind 和 nodeval
    tree->stepIn("Variable");
    tree->stepIn("ID");
    TreeNode* p = tree->now;
    string var_idname = tree->now->tk->wd.str;
    Symbol* varsym = nullptr;
    bool ifVarNotInTable = false;
    int scopenum = scope.size();
    bool findNearSameLevel = false;
    //int scopenum = levelCurrent;//且很关键的一点，
    for(int i = scopenum-1; i >= 0; i--)
    {
        SymbolTable* tempsymtbl = scope[i];
        if(tempsymtbl->table[0]->level+1 > levelCurrent) continue;
        if(tempsymtbl->table[0]->level+1 == levelCurrent){
            if(!findNearSameLevel)
                findNearSameLevel = true;
            else continue;
        }
        if(tempsymtbl->findByName(var_idname)){
            varsym = tempsymtbl->findByName(var_idname);
            tree->now->semantic = varsym;
            break;
        }
    }
    if(varsym == nullptr){
        ifVarNotInTable = true;
        UndefinedSymbol e(var_idname, p->tk->line);
        cout << e.what() << endl;//ID 未定义使用
    }
    tree->stepIn("VariMore");
    if(tree->now->child.empty()){
        if(ifVarNotInTable)   //未定错误
            return nullptr;
        Symbol* tmpvarsym = new Symbol(varsym->name, varsym->decKind, varsym->tp, p->tk);
        return tmpvarsym; // 否则返回找到的ID对象 Varimore -> ε
    }
    //    VariMore -> ε | [ Exp ] | . FieldVar
    tree->preorderStep();
    string varimore_choice = tree->now->name;
    if(varimore_choice == "LMIDPAREN"){ //当然这里可以直接返回全部的Exp
        Symbol* array_index = synaxExpression();
        string arr_index_type = array_index->getType();
        string arr_index_value = array_index->tk->wd.str;
        int arr_index_lineno = array_index->tk->line;
        if(arr_index_type == "INTEGER" || arr_index_type == "INTC"){
            if(arr_index_type == "INTC"){
                int indexval = stoi(arr_index_value);
                int definelow = varsym->tp->at->low;
                int definehigh = varsym->tp->at->top;
                if(indexval >= definelow && indexval <= definehigh){
                    string CorrectMes = "Array access correctly in line " + to_string(arr_index_lineno);
                    for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
                    cout << CorrectMes << endl;
                    runMessgae.push_back(CorrectMes);
                }
                else{
                    ArrayIndexOutBound e(p->name, arr_index_lineno, definelow, definehigh);
                    cout << e.what() << endl;
                }
            }
            else{
                string CorrectMes = "Array access correctly in line " + to_string(arr_index_lineno);
                for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
                cout << CorrectMes << endl;
                runMessgae.push_back(CorrectMes);
            }
        }
        else{
            ArrayIndexMisMatch e(p->name, arr_index_lineno);
            cout << e.what() << endl;
        }
        return array_index;
    }
    else if(varimore_choice == "DOT"){
        cout << "暂时放弃" << endl;
        assert(false);
    }
    else assert(false);

}

//ConditionalStm -> IF RelExp THEN StmList ELSE StmList FI
void Analyzer::synaxConditionalStm() {
    tree->stepIn("IF");
    Symbol* conditiaon_exp = synaxRelExpression();
    tree->stepIn("THEN");
    synaxStmList();
    tree->stepIn("ELSE");
    synaxStmList();
    tree->stepIn("FI");
    TreeNode* p = tree->now;
    string CorrectMes = "ConditionalStm run correctly in line " + to_string(p->tk->line);
    if(SEMANTICDEBUG) cout << CorrectMes <<endl;
    runMessgae.push_back(CorrectMes);
}

void Analyzer::synaxLoopStm() {
    //LoopStm -> WHILE RelExp DO StmList ENDWH
    tree->stepIn("WHILE");
    Symbol* conditiaon_exp = synaxRelExpression();
    tree->stepIn("DO");
    synaxStmList();
    tree->stepIn("ENDWH");
    TreeNode* p = tree->now;
    string CorrectMes = "LoopStm run correctly in line " + to_string(p->tk->line);
    if(SEMANTICDEBUG) cout << CorrectMes <<endl;
    runMessgae.push_back(CorrectMes);
}

void Analyzer::synaxReturnStm() {
    tree->stepIn("ReturnStm");
    tree->stepIn("RETURN");
    TreeNode* p = tree->now;
    string CorrectMes = "ReturnStm run correctly in line " + to_string(p->tk->line);
    runMessgae.push_back(CorrectMes);
    if(SEMANTICDEBUG) cout << CorrectMes <<endl;

}

//RelExp -> Exp OtherRelE
//OtherRelE -> CmpOp Exp
//CmpOp -> < | =
Symbol* Analyzer::synaxRelExpression() {
    tree->stepIn("RelExp");
    TreeNode* p = tree->now;
    Symbol* left_expsym = synaxExpression();
    string left_exptype = left_expsym->getType();
    int currentline = left_expsym->tk->line;
    tree->stepIn("CmpOp");
    Symbol* right_expsym = synaxExpression();
    string right_exptype = right_expsym->getType();
    if(!assign_typecheck(left_exptype, right_exptype)){
        CompareMisMatch e(left_exptype, right_exptype, p->tk->line);
        cout << e.what() << endl;
        return nullptr;
    }
    else{
        string CorrectMes = "RelExpression run correctly in line " + to_string(currentline);
        for(int i = 0; i < tbSym->table[0]->level; i++) cout << '\t';
        cout << CorrectMes << endl;
        runMessgae.push_back(CorrectMes);
        return left_expsym;
    }
}

bool Analyzer::assign_typecheck(string lefttype, string righttype) {
    if(lefttype == "INTEGER"){
        return righttype == "INTEGER" || righttype == "INTC";
    }
    else if(lefttype == "INTC"){
        return righttype == "INTEGER" || righttype == "INTC";
    }
    else {
        return lefttype == righttype;
    }
}

void Analyzer::getSemanticList(TreeNode *p) {
    if(p->semantic){
        assert(p->semantic);
        semanticList.push_back(p->semantic);
    }
    int m = p->child.size();
    for(int i = 0; i < m; i++){
        getSemanticList(p->child[i]);
    }
    return ;
}

string nodeKindToString(NodeKind x){
    switch (x) {
        case ProK:  return "ProK";
        case PheadK: return "PheadK";
        case TypeK: return "TypeK";
        case VarK:   return "VarK";
        case ProcDecK:  return "ProcDecK";
        case StmLK: return "StmLK";
        case DecK:  return "DecK";
        case StmtK: return "StmtK";
        case ExpK: return "ExpK";
        default:
            assert(false);
            return "Unknown NodeKind";
    }
}

void printSymbolTable(SymbolTable *symtbl) {
    int m = symtbl->table.size();
    for(int i = 0; i < m; i++){
        symtbl->table[i]->printSymbol();
    }
}

