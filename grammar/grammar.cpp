#include "global.h"
#include "lexical.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include "grammar.h"
using namespace std;

extern Token tokenList[1010];
int pointer = 0;
GrammarTree* tree;

#define LEXOPENFILEDIR "../Reference/lexicalinputtest.txt"
#define LEXOUTPUTFILEDIR "../lexicaloutput.txt"

#define GRAOUTPUTFILEDIR "../grammaroutput.txt"


#define GRAMMARMAIN
#ifdef GRAMMARMAIN
int main(){
    initReservedWord(); //初始化保留字
    FILE *fp;
    std::ofstream outputFile(GRAOUTPUTFILEDIR, std::ios::out | std::ios::binary);
    if(outputFile.is_open()){
        outputFile << u8"\uFEFF"; // 写入 BOM (Byte Order Mark)，表示文件以 UTF-8 编码
    }

    freopen(GRAOUTPUTFILEDIR, "w", stdout); // 输出重定向
    fp = fopen(LEXOPENFILEDIR, "r");
    fflush(stdout);
    lexicalAnalyse(fp);
//    printTokenList();
    TreeNode* t = program();

    cout << 1 << endl;  //奇怪的重定问题

    if (tokenList[pointer].wd.tok == ENDFILE) {
        cout << "success!" << endl;
    }
//    dfsPrintTree(t,0);
    printToPy(t, 0);
    fclose(fp);
}
#endif

void dfsPrintTree(TreeNode* p, int level){
    int tmp = level;
    while(tmp--)    cout << '\t';
    cout << p->name;
    if(p->ifTermimal == "VT"){
        cout  << ' '<< (p->tk->wd.str);
    }
    cout << endl;
    int m = p->child.size();
    for(int i = 0; i < m ; i++)
    {
        dfsPrintTree(p->child[i],level+1);
    }
    return ;
}

void printToPy(TreeNode* p, int level){
    int tmp = level;
    while(tmp--)    cout << '\t';
    if(p->ifTermimal == "VT"){
        cout << '<' << "VT|" << p->tk->wd.str << '|' << p->child.size() << '>';
    }
    else{
        cout << '<' << "VN|" << p->name << '|' << p->child.size() << '>';
    }
    cout << endl;
    int m = p->child.size();
    for(int i = 0; i < m ; i++)
    {
        printToPy(p->child[i],level+1);
    }
    return ;
}


void grammarerror(int lineNo, string origin, string now){
    try {
        throw SyntaxError(lineNo, origin, now);
    }catch (const SyntaxError& e){
        cout << e.what() << endl;
    }
}

LexType getCurLex(int p){
    return tokenList[p].wd.tok;
}
int getCurLine(int p){
    return tokenList[p].line;
}

TreeNode* matchToken(LexType lex){
    if(getCurLex(pointer) == lex){
        TreeNode* p = new TreeNode(enumToString(lex), &tokenList[pointer], "VT");
        pointer++;
        return p;
    }
    else{
        grammarerror(tokenList[pointer].line, enumToString(lex), enumToString(getCurLex(pointer)));
    }
}

//1. Program -> ProgramHead DeclarePart ProgramBody .    PROGRAM
TreeNode* program() { //开始程序！
    pointer = 0;
    tree = new GrammarTree();
    TreeNode* root = nullptr;
    if (getCurLex(pointer) == PROGRAM) {
        root = new TreeNode("Program", nullptr, "VN");
        root->addChild(programHead());
        root->addChild(declarePart());
        root->addChild(programBody());
        root->addChild(matchToken(DOT));
    }
    else {
        grammarerror(tokenList[pointer].line, "Program", enumToString(getCurLex(pointer)));
    }
    return root;
}

//2. ProgramHead -> PROGRAM ProgramName
TreeNode* programHead() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == PROGRAM) {
        t = new TreeNode("ProgramHead", nullptr, "VN");
        t->addChild(matchToken(PROGRAM));
        t->addChild(programName());
    }
    else {
        grammarerror(getCurLine(pointer),"ProgramHead", enumToString(getCurLex(pointer)));
    }
    return t;
}

//3. ProgramName -> ID
TreeNode* programName() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("ProgramName", nullptr, "VN");
        t->addChild(matchToken(ID));
    }
    else { grammarerror(getCurLine(pointer),"ProgramHead",enumToString(getCurLex(pointer))); }
    return t;
}

//4. DeclarePart -> TypeDec VarDec ProcDec
TreeNode* declarePart() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == TYPE || getCurLex(pointer) == VAR || getCurLex(pointer) == PROCEDURE || getCurLex(pointer) == BEGIN) {
        t = new TreeNode("DeclarePart", nullptr, "VN");
        t->addChild(typeDec());
        t->addChild(varDec());
        t->addChild(procDec());
    }
    else {
        grammarerror(getCurLine(pointer),"DeclarePart",enumToString(getCurLex(pointer)));
    }
    return t;
}


//6. TypeDec -> ε | TypeDeclaration
TreeNode* typeDec() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == VAR || getCurLex(pointer) == PROCEDURE || getCurLex(pointer) == BEGIN) {
        t = new TreeNode("TypeDec", nullptr, "VN");
    }
    else if (getCurLex(pointer) == TYPE) {
        t = new TreeNode("TypeDec", nullptr, "VN");
        t->addChild(typeDeclaration());
    }
    else { grammarerror(getCurLine(pointer), "TypeDec",enumToString(getCurLex(pointer))); }
    return t;
}


//7. TypeDeclaration ->  TYPE	TypeDecList
TreeNode* typeDeclaration() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == TYPE) {
        t = new TreeNode("TypeDeclaration", nullptr, "VN");
        t->addChild(matchToken(TYPE));
        t->addChild(typeDecList());
    }
    else { grammarerror(getCurLine(pointer), "TypeDeclaration",enumToString(getCurLex(pointer))); }
    return t;
}

//8. TypeDecList -> TypeId = TypeName ; TypeDecMore
TreeNode* typeDecList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("TypeDecList", nullptr, "VN");
        t->addChild(typeID());           //TypeId
        t->addChild(matchToken(EQ));
        t->addChild(typeName());
        t->addChild(matchToken(SEMI));
        t->addChild(typeDecMore());
    }
    else { grammarerror(getCurLine(pointer),"TypeDecList",enumToString(getCurLex(pointer))); }
    return t;
}


//9. TypeDecMore -> ε | TypeDecList
TreeNode* typeDecMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == VAR || getCurLex(pointer) == PROCEDURE || getCurLex(pointer) == BEGIN) {
        t = new TreeNode("TypeDecMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == ID) {
        t = new TreeNode("TypeDecMore", nullptr, "VN");
        t->addChild(typeDecList());
    }
    else { grammarerror(getCurLine(pointer),"TypeDecMore",enumToString(getCurLex(pointer))); }
    return t;
}

//11. TypeId -> ID
TreeNode* typeID() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("TypeID", nullptr, "VN");
        t->addChild(matchToken(ID));
    }
    else { grammarerror(getCurLine(pointer),"TypeID",enumToString(getCurLex(pointer))); }
    return t;
}

//12 TypeName -> BaseType | StructureType | ID
TreeNode* typeName() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR) {
        t = new TreeNode("TypeName", nullptr, "VN");
        t->addChild(baseType());
    }
    else if (getCurLex(pointer) == ARRAY || getCurLex(pointer) == RECORD) {
        t = new TreeNode("TypeName", nullptr, "VN");
        t->addChild(structureType());
    }
    else if (getCurLex(pointer) == ID) {
        t = new TreeNode("TypeName", nullptr, "VN");
        t->addChild(matchToken(ID));
    }
    else { grammarerror(getCurLine(pointer), "TypeName",enumToString(getCurLex(pointer))); }
    return t;
}

//15. BaseType -> INTEGER | CHAR
TreeNode* baseType() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTEGER) {
        t = new TreeNode("BaseType", nullptr, "VN");
        t->addChild(matchToken(INTEGER));
    }
    else if (getCurLex(pointer) == CHAR) {
        t = new TreeNode("BaseType", nullptr, "VN");
        t->addChild(matchToken(CHAR));
    }
    else {
        grammarerror(getCurLine(pointer),"BaseType",enumToString(getCurLex(pointer)));
    }
    return t;
}

//17.StructureType -> ArrayType | RecType
TreeNode* structureType() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ARRAY) {
        t = new TreeNode("StructureType", nullptr, "VN");
        t->addChild(arrayType());
    }
    else if (getCurLex(pointer) == RECORD) {
        t = new TreeNode("StructureType", nullptr, "VN");
        t->addChild(recType());
    }
    else {
        grammarerror(getCurLine(pointer),"StructureType",enumToString(getCurLex(pointer)));
    }
    return t;
}

//19.ArrayType -> ARRAY [ Low .. Top ] OF BaseType
TreeNode* arrayType() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ARRAY) {
        t = new TreeNode("ArrayType", nullptr, "VN");
        t->addChild(matchToken(ARRAY));
        t->addChild(matchToken(LMIDPAREN));
        t->addChild(low());
        t->addChild(matchToken(UNDERANGE));
        t->addChild(top());
        t->addChild(matchToken(RMIDPAREN));
        t->addChild(matchToken(OF));
        t->addChild(baseType());
    }
    else {
        grammarerror(getCurLine(pointer),"ArrayType",enumToString(getCurLex(pointer)));

    }
    return t;
}

//20.Low -> INTC
TreeNode* low() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTC) {
        t = new TreeNode("Low", nullptr, "VN");
        t->addChild(matchToken(INTC));
    }
    else {
        grammarerror(getCurLine(pointer),"Low",enumToString(getCurLex(pointer)));
    }
    return t;
}

//21.Top -> INTC
TreeNode* top() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTC) {
        t = new TreeNode("Top", nullptr, "VN");
        t->addChild(matchToken(INTC));
    }
    else {
        grammarerror(getCurLine(pointer),"Top",enumToString(getCurLex(pointer)));
    }
    return t;
}

//22.RecType -> RECORD FieldDecList END
TreeNode* recType() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == RECORD) {
        t = new TreeNode("RecType", nullptr, "VN");
        t->addChild(matchToken(RECORD));
        t->addChild(fieldDecList());
        t->addChild(matchToken(END1));
    }
    else { grammarerror(getCurLine(pointer),"RecType",enumToString(getCurLex(pointer))); }
    return t;
}

//23.FieldDecList -> BaseType IdList ; FieldDecMore | ArrayType IdList ; FieldDecMore
TreeNode* fieldDecList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR) {
        t = new TreeNode("FieldDecList", nullptr, "VN");
        t->addChild(baseType());
        t->addChild(IDList());
        t->addChild(matchToken(SEMI));
        t->addChild(fieldDecMore());
    }
    else if (getCurLex(pointer) == ARRAY) {
        t = new TreeNode("FieldDecList", nullptr, "VN");
        t->addChild(arrayType());
        t->addChild(IDList());
        t->addChild(matchToken(SEMI));
        t->addChild(fieldDecMore());
    }
    else { grammarerror(getCurLine(pointer),"FieldDecList",enumToString(getCurLex(pointer))); }
    return t;
}

//25. FieldDecMore -> ε | FieldDecList
TreeNode* fieldDecMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == END1) {
        t = new TreeNode("FieldDecMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR || getCurLex(pointer) == ARRAY) {
        t = new TreeNode("FieldDecMore", nullptr, "VN");
        t->addChild(fieldDecList());
    }
    else { grammarerror(getCurLine(pointer),"FieldDecMore",enumToString(getCurLex(pointer))); }
    return t;
}

//27.IdList -> ID IdMore
TreeNode* IDList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("IDList", nullptr, "VN");
        t->addChild(matchToken(ID));
        t->addChild(IDMore());
    }
    else { grammarerror(getCurLine(pointer), "IDList",enumToString(getCurLex(pointer))); }
    return t;
}

//28.IdMore -> ε | , IdList
TreeNode* IDMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == SEMI) {
        t = new TreeNode("IDMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == COMMA) {
        t = new TreeNode("IDMore", nullptr, "VN");
        t->addChild(matchToken(COMMA));
        t->addChild(IDList());
    }
    else { grammarerror(getCurLine(pointer), "IDMore",enumToString(getCurLex(pointer))); }
    return t;
}

//30.VarDec -> ε | VarDeclaration
TreeNode* varDec() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == PROCEDURE || getCurLex(pointer) == BEGIN) {
        t = new TreeNode("VarDec", nullptr, "VN");
    }
    else if (getCurLex(pointer) == VAR) {
        t = new TreeNode("VarDec", nullptr, "VN");
        t->addChild(varDeclaration());
    }
    else { grammarerror(getCurLine(pointer), "VarDec",enumToString(getCurLex(pointer))); }
    return t;
}

//32. VarDeclaration -> VAR VarDecList
TreeNode* varDeclaration() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == VAR) {
        t = new TreeNode("VarDeclaration", nullptr, "VN");
        t->addChild(matchToken(VAR));
        t->addChild(varDecList());
    }
    else { grammarerror(getCurLine(pointer), "VarDeclaration",enumToString(getCurLex(pointer))); }
    return t;
}

//33.VarDecList -> TypeName VarIdList ; VarDecMore
TreeNode* varDecList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR || getCurLex(pointer) == ARRAY ||
        getCurLex(pointer) == RECORD || getCurLex(pointer) == ID) {
        t = new TreeNode("VarDecList", nullptr, "VN");
        t->addChild(typeName());
        t->addChild(varIDList());
        t->addChild(matchToken(SEMI));
        t->addChild(varDecMore());
    }
    else { grammarerror(getCurLine(pointer), "VarDecList",enumToString(getCurLex(pointer))); }
    return t;
}

//34.VarDecMore -> ε | VarDecList
TreeNode* varDecMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == PROCEDURE || getCurLex(pointer) == BEGIN) {
        t = new TreeNode("VarDecMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR || getCurLex(pointer) == ARRAY ||
             getCurLex(pointer) == RECORD || getCurLex(pointer) == ID) {
        t = new TreeNode("VarDecMore", nullptr, "VN");
        t->addChild(varDecList());
    }
    else { grammarerror(getCurLine(pointer), "VarDecMore",enumToString(getCurLex(pointer))); }
    return t;
}

//36.VarIdList -> ID VarIdMore
TreeNode* varIDList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("VarIDList", nullptr, "VN");
        t->addChild(matchToken(ID));
        t->addChild(varIDMore());
    }
    else { grammarerror(getCurLine(pointer), "VarIDList",enumToString(getCurLex(pointer))); }
    return t;
}

//37.VarIdMore -> ε | , VarIdList
TreeNode* varIDMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == SEMI) {
        t = new TreeNode("VarIDMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == COMMA) {
        t = new TreeNode("VarIDMore", nullptr, "VN");
        t->addChild(matchToken(COMMA));
        t->addChild(varIDList());
    }
    else { grammarerror(getCurLine(pointer), "VarIDMore",enumToString(getCurLex(pointer))); }
    return t;
}

//39. ProcDec -> ε | ProcDeclaration
TreeNode* procDec() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == BEGIN) {
        t = new TreeNode("ProcDec", nullptr, "VN");
    }
    else if (getCurLex(pointer) == PROCEDURE) {
        t = new TreeNode("ProcDec", nullptr, "VN");
        t->addChild(procDeclaration());
    }
    else { grammarerror(getCurLine(pointer), "ProcDec",enumToString(getCurLex(pointer))); }
    return t;
}

//41.ProcDeclaration -> PROCEDURE ProcName ( ParamList ) ; ProcDecPart ProcBody ProcDecMore
//procDecMore 和 procDec完全相同
TreeNode* procDeclaration() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == PROCEDURE) {
        t = new TreeNode("ProcDeclaration", nullptr, "VN");
        t->addChild(matchToken(PROCEDURE));
        t->addChild(procName());
        t->addChild(matchToken(LPAREN));
        t->addChild(paramList());
        t->addChild(matchToken(RPAREN));
        t->addChild(matchToken(SEMI));
        t->addChild(procDecPart());
        t->addChild(procBody());
        t->addChild(procDec());
    }
    else { grammarerror(getCurLine(pointer), "ProcDeclaration",enumToString(getCurLex(pointer))); }
    return t;
}

//42.ProcDecMore ->	ε  | ProcDeclaration
//由于procDecMore 和 procDecPart完全相同 直接代替


//44. ProcName -> ID
TreeNode* procName() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("ProcName", nullptr, "VN");
        t->addChild(matchToken(ID));
    }
    else { grammarerror(getCurLine(pointer), "ProcName",enumToString(getCurLex(pointer))); }
    return t;
}

//45. ParamList -> ε | ParamDecList
TreeNode* paramList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == RPAREN) {
        t = new TreeNode("ParamList", nullptr, "VN");
    }
    else if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR || getCurLex(pointer) == ARRAY ||
             getCurLex(pointer) == RECORD || getCurLex(pointer) == ID || getCurLex(pointer) == VAR) {
        t = new TreeNode("ParamList", nullptr, "VN");
        t->addChild(paramDecList());
    }
    else { grammarerror(getCurLine(pointer), "ParamList",enumToString(getCurLex(pointer))); }
    return t;
}

//47. ParamDecList -> Param ParamMore
TreeNode* paramDecList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR || getCurLex(pointer) == ARRAY ||
        getCurLex(pointer) == RECORD || getCurLex(pointer) == ID || getCurLex(pointer) == VAR) {
        t = new TreeNode("ParamDecList", nullptr, "VN");
        t->addChild(param());
        t->addChild(paramMore());
    }
    else { grammarerror(getCurLine(pointer), "ParamDecList",enumToString(getCurLex(pointer))); }
    return t;
}

//48. ParamMore -> ε | ; ParamDecList
TreeNode* paramMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == RPAREN) {
        t = new TreeNode("ParamMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == SEMI) {
        t = new TreeNode("ParamMore", nullptr, "VN");
        t->addChild(matchToken(SEMI));
        t->addChild(paramDecList());
    }
    else { grammarerror(getCurLine(pointer), "ParamMore",enumToString(getCurLex(pointer))); }
    return t;
}

//50. Param -> TypeName FormList | VAR TypeName FormList
TreeNode* param() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == INTEGER || getCurLex(pointer) == CHAR || getCurLex(pointer) == ARRAY ||
        getCurLex(pointer) == RECORD || getCurLex(pointer) == ID) {
        t = new TreeNode("Param", nullptr, "VN");
        t->addChild(typeName());
        t->addChild(formList());
    }
    else if (getCurLex(pointer) == VAR) {
        t = new TreeNode("Param", nullptr, "VN");
        t->addChild(matchToken(VAR));
        t->addChild(typeName());
        t->addChild(formList());
    }
    else { grammarerror(getCurLine(pointer), "Param",enumToString(getCurLex(pointer))); }
    return t;
}

//52. FormList -> ID FidMore
TreeNode* formList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("FormList", nullptr, "VN");
        t->addChild(matchToken(ID));
        t->addChild(fidMore());
    }
    else { grammarerror(getCurLine(pointer), "FormList",enumToString(getCurLex(pointer))); }
    return t;
}

//53. FidMore -> ε | , FormList
TreeNode* fidMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == SEMI || getCurLex(pointer) == RPAREN) {
        t = new TreeNode("FidMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == COMMA) {
        t = new TreeNode("FidMore", nullptr, "VN");
        t->addChild(matchToken(COMMA));
        t->addChild(formList());
    }
    else { grammarerror(getCurLine(pointer), "FidMore",enumToString(getCurLex(pointer))); }
    return t;
}


//55 ProcDecPart ->	DeclarePart
TreeNode* procDecPart() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == TYPE || getCurLex(pointer) == VAR || getCurLex(pointer) == PROCEDURE ||
        getCurLex(pointer) == BEGIN) {
        t = new TreeNode("ProDecPart", nullptr, "VN");
        t->addChild(declarePart());
    }
    else { grammarerror(getCurLine(pointer), "ProDecPart",enumToString(getCurLex(pointer))); }
    return t;
}


//56. ProcBody -> ProgramBody
TreeNode* procBody() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == BEGIN) {
        t = new TreeNode("ProcBody", nullptr, "VN");
        t->addChild(programBody());
    }
    else { grammarerror(getCurLine(pointer), "ProcBody",enumToString(getCurLex(pointer))); }
    return t;
}

//57. ProgramBody -> BEGIN StmList END
TreeNode* programBody() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == BEGIN) {
        t = new TreeNode("ProgramBody", nullptr, "VN");
        t->addChild(matchToken(BEGIN));
        t->addChild(stmList());
        t->addChild(matchToken(END1));
    }
    else { grammarerror(getCurLine(pointer), "ProgramBody",enumToString(getCurLex(pointer))); }
    return t;
}

//58. StmList -> Stm StmMore
TreeNode* stmList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID || getCurLex(pointer) == IF || getCurLex(pointer) == WHILE ||
        getCurLex(pointer) == RETURN1 || getCurLex(pointer) == READ || getCurLex(pointer) == WRITE) {
        t = new TreeNode("StmList", nullptr, "VN");
        t->addChild(stm());
        t->addChild(stmMore());
    }
    else { grammarerror(getCurLine(pointer), "StmList",enumToString(getCurLex(pointer))); }
    return t;
}

//59. StmMore -> ε | ; StmList
TreeNode* stmMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ELSE || getCurLex(pointer) == FI || getCurLex(pointer) == END1 ||
        getCurLex(pointer) == ENDWH) {
        t = new TreeNode("StmMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == SEMI) {
        t = new TreeNode("StmMore", nullptr, "VN");
        t->addChild(matchToken(SEMI));
        t->addChild(stmList());
    }
    else { grammarerror(getCurLine(pointer), "StmMore",enumToString(getCurLex(pointer))); }
    return t;
}

//61. Stm -> ConditionalStm | LoopStm | InputStm | OutputStm | ReturnStm | ID AssCall
TreeNode* stm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == IF) {
        t = new TreeNode("Stm", nullptr, "VN");
        t->addChild(conditionalStm());
    }
    else if (getCurLex(pointer) == WHILE) {
        t = new TreeNode("Stm", nullptr, "VN");
        t->addChild(loopStm());
    }
    else if (getCurLex(pointer) == READ) {
        t = new TreeNode("Stm", nullptr, "VN");
        t->addChild(inputStm());
    }
    else if (getCurLex(pointer) == WRITE) {
        t = new TreeNode("Stm", nullptr, "VN");
        t->addChild(outputStm());
    }
    else if (getCurLex(pointer) == RETURN1) {
        t = new TreeNode("Stm", nullptr, "VN");
        t->addChild(returnStm());
    }
    else if (getCurLex(pointer) == ID) {
        t = new TreeNode("Stm", nullptr, "VN");
        t->addChild(matchToken(ID));
        t->addChild(assCall());
    }
    else { grammarerror(getCurLine(pointer), "Stm",enumToString(getCurLex(pointer))); }
    return t;
}

//67. AssCall -> AssignmentRest | CallStmRest
TreeNode* assCall() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ASSIGN || getCurLex(pointer) == LMIDPAREN
        || getCurLex(pointer) == DOT) {
        t = new TreeNode("AssCall", nullptr, "VN");
        t->addChild(assignmentRest());
    }
    else if (getCurLex(pointer) == LPAREN) {
        t = new TreeNode("AssCall", nullptr, "VN");
        t->addChild(callStmRest());
    }
    else { grammarerror(getCurLine(pointer), "AssCall",enumToString(getCurLex(pointer))); }
    return t;
}

//69. AssignmentRest -> VariMore := Exp
TreeNode* assignmentRest() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LMIDPAREN || getCurLex(pointer) == DOT || getCurLex(pointer) == ASSIGN) {
        t = new TreeNode("AssignmentRest", nullptr, "VN");
        t->addChild(variMore());
        t->addChild(matchToken(ASSIGN));
        t->addChild(exp());
    }
    else { grammarerror(getCurLine(pointer), "AssignmentRest",enumToString(getCurLex(pointer))); }
    return t;
}

//70. ConditionalStm -> IF RelExp THEN StmList ELSE StmList FI
TreeNode* conditionalStm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == IF) {
        t = new TreeNode("ConditionalStm", nullptr, "VN");
        t->addChild(matchToken(IF));
        t->addChild(relExp());
        t->addChild(matchToken(THEN));
        t->addChild(stmList());
        t->addChild(matchToken(ELSE));
        t->addChild(stmList());
        t->addChild(matchToken(FI));
    }
    else { grammarerror(getCurLine(pointer), "ConditionalStm",enumToString(getCurLex(pointer))); }
    return t;
}

//71. LoopStm -> WHILE RelExp DO StmList ENDWH
TreeNode* loopStm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == WHILE) {
        t = new TreeNode("LoopStm", nullptr, "VN");
        t->addChild(matchToken(WHILE));
        t->addChild(relExp());
        t->addChild(matchToken(DO));
        t->addChild(stmList());
        t->addChild(matchToken(ENDWH));
    }
    else { grammarerror(getCurLine(pointer), "LoopStm",enumToString(getCurLex(pointer))); }
    return t;
}

//72. InputStm -> READ ( Invar )
TreeNode* inputStm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == READ) {
        t = new TreeNode("InputStm", nullptr, "VN");
        t->addChild(matchToken(READ));
        t->addChild(matchToken(LPAREN));
        t->addChild(inVar());
        t->addChild(matchToken(RPAREN));
    }
    else { grammarerror(getCurLine(pointer), "InputStm",enumToString(getCurLex(pointer))); }
    return t;
}

//73. Invar -> ID
TreeNode* inVar() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("InVar", nullptr, "VN");
        t->addChild(matchToken(ID));
    }
    else { grammarerror(getCurLine(pointer), "InVar",enumToString(getCurLex(pointer))); }
    return t;
}

//74. OutputStm -> WRITE ( Exp )
TreeNode* outputStm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == WRITE) {
        t = new TreeNode("OutputStm", nullptr, "VN");
        t->addChild(matchToken(WRITE));
        t->addChild(matchToken(LPAREN));
        t->addChild(exp());
        t->addChild(matchToken(RPAREN));
    }
    else { grammarerror(getCurLine(pointer), "OutputStm",enumToString(getCurLex(pointer))); }
    return t;
}

//75. ReturnStm -> RETURN ( Exp )
TreeNode* returnStm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == RETURN1) {
        t = new TreeNode("ReturnStm", nullptr, "VN");
        t->addChild(matchToken(RETURN1));
    }
    else { grammarerror(getCurLine(pointer), "ReturnStm",enumToString(getCurLex(pointer))); }
    return t;
}

//76. CallStmRest -> ( ActParamList )
TreeNode* callStmRest() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LPAREN) {
        t = new TreeNode("CallStmRest", nullptr, "VN");
        t->addChild(matchToken(LPAREN));
        t->addChild(actparamList());
        t->addChild(matchToken(RPAREN));
    }
    else { grammarerror(getCurLine(pointer), "CallStmRest",enumToString(getCurLex(pointer))); }
    return t;
}

//77. ActParamList -> ε | Exp ActParamMore
TreeNode* actparamList() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == RPAREN) {
        t = new TreeNode("ActparamList", nullptr, "VN");
    }
    else if (getCurLex(pointer) == LPAREN || getCurLex(pointer) == INTC || getCurLex(pointer) == ID) {
        t = new TreeNode("ActparamList", nullptr, "VN");
        t->addChild(exp());
        t->addChild(actparamMore());
    }
    else { grammarerror(getCurLine(pointer), "ActparamList",enumToString(getCurLex(pointer))); }
    return t;
}

//79. ActParamMore -> ε | , ActParamList
TreeNode* actparamMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == RPAREN) {
        t = new TreeNode("ActparamMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == COMMA) {
        t = new TreeNode("ActparamMore", nullptr, "VN");
        t->addChild(matchToken(COMMA));
        t->addChild(actparamList());
    }
    else { grammarerror(getCurLine(pointer), "ActparamMore",enumToString(getCurLex(pointer))); }
    return t;
}

//81. RelExp -> Exp OtherRelE
TreeNode* relExp() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LPAREN || getCurLex(pointer) == INTC || getCurLex(pointer) == ID) {
        t = new TreeNode("RelExp", nullptr, "VN");
        t->addChild(exp());
        t->addChild(otherRelE());
    }
    else { grammarerror(getCurLine(pointer), "RelExp",enumToString(getCurLex(pointer))); }
    return t;
}

//82. OtherRelE -> CmpOp Exp
TreeNode* otherRelE() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LT || getCurLex(pointer) == EQ) {
        t = new TreeNode("OtherRelE", nullptr, "VN");
        t->addChild(cmpOp());
        t->addChild(exp());
    }
    else { grammarerror(getCurLine(pointer), "OtherRelE",enumToString(getCurLex(pointer))); }
    return t;
}

//83. Exp -> Term OtherTerm
TreeNode* exp() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LPAREN || getCurLex(pointer) == INTC || getCurLex(pointer) == ID) {
        t = new TreeNode("Exp", nullptr, "VN");
        t->addChild(term());
        t->addChild(otherTerm());
    }
    else { grammarerror(getCurLine(pointer), "Exp",enumToString(getCurLex(pointer))); }
    return t;
}


//84. OtherTerm -> ε | AddOp Exp
TreeNode* otherTerm() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LT || getCurLex(pointer) == EQ || getCurLex(pointer) == RMIDPAREN ||
        getCurLex(pointer) == THEN || getCurLex(pointer) == ELSE || getCurLex(pointer) == FI ||
        getCurLex(pointer) == DO || getCurLex(pointer) == ENDWH || getCurLex(pointer) == RPAREN ||
        getCurLex(pointer) == END1 || getCurLex(pointer) == SEMI || getCurLex(pointer) == COMMA) {
        t = new TreeNode("OtherTerm", nullptr, "VN");
    }
    else if (getCurLex(pointer) == PLUS || getCurLex(pointer) == MINUS) {
        t = new TreeNode("OtherTerm", nullptr, "VN");
        t->addChild(addOp());
        t->addChild(exp());
    }
    else { grammarerror(getCurLine(pointer), "OtherTerm",enumToString(getCurLex(pointer))); }
    return t;
}

//86. Term -> Factor OtherFactor
TreeNode* term() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LPAREN || getCurLex(pointer) == INTC || getCurLex(pointer) == ID) {
        t = new TreeNode("Term", nullptr, "VN");
        t->addChild(factor());
        t->addChild(otherFactor());
    }
    else { grammarerror(getCurLine(pointer), "Term",enumToString(getCurLex(pointer))); }
    return t;
}

//87. OtherFactor  ->  ε    |  MultOp  Term
TreeNode* otherFactor() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == PLUS || getCurLex(pointer) == MINUS || getCurLex(pointer) == LT ||
        getCurLex(pointer) == EQ || getCurLex(pointer) == RMIDPAREN || getCurLex(pointer) == THEN ||
        getCurLex(pointer) == ELSE || getCurLex(pointer) == FI || getCurLex(pointer) == DO ||
        getCurLex(pointer) == ENDWH || getCurLex(pointer) == RPAREN || getCurLex(pointer) == END1 ||
        getCurLex(pointer) == SEMI || getCurLex(pointer) == COMMA) {
        t = new TreeNode("OtherFactor", nullptr, "VN");
    }
    else if (getCurLex(pointer) == TIMES || getCurLex(pointer) == OVER) {
        t = new TreeNode("OtherFactor", nullptr, "VN");
        t->addChild(multOp());
        t->addChild(term());
    }
    else { grammarerror(getCurLine(pointer), "OtherFactor",enumToString(getCurLex(pointer))); }
    return t;
}

//89. Factor -> ( Exp ) | INTC | Variable
TreeNode* factor() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LPAREN) {
        t = new TreeNode("Factor", nullptr, "VN");
        t->addChild(matchToken(LPAREN));
        t->addChild(exp());
        t->addChild(matchToken(RPAREN));
    }
    else if (getCurLex(pointer) == INTC) {
        t = new TreeNode("Factor", nullptr, "VN");
        t->addChild(matchToken(INTC));
    }
    else if (getCurLex(pointer) == ID) {
        t = new TreeNode("Factor", nullptr, "VN");
        t->addChild(variable());
    }
    else { grammarerror(getCurLine(pointer), "Factor",enumToString(getCurLex(pointer))); }
    return t;
}

//92. Variable -> ID VariMore
TreeNode* variable() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("Variable", nullptr, "VN");
        t->addChild(matchToken(ID));
        t->addChild(variMore());
    }
    else { grammarerror(getCurLine(pointer), "Variable",enumToString(getCurLex(pointer))); }
    return t;
}

//93. VariMore -> ε | [ Exp ] | . FieldVar
TreeNode* variMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ASSIGN || getCurLex(pointer) == TIMES || getCurLex(pointer) == OVER ||
        getCurLex(pointer) == PLUS || getCurLex(pointer) == MINUS || getCurLex(pointer) == LT ||
        getCurLex(pointer) == EQ || getCurLex(pointer) == THEN || getCurLex(pointer) == ELSE ||
        getCurLex(pointer) == FI || getCurLex(pointer) == DO || getCurLex(pointer) == ENDWH ||
        getCurLex(pointer) == RPAREN || getCurLex(pointer) == END1 || getCurLex(pointer) == SEMI ||
        getCurLex(pointer) == COMMA || getCurLex(pointer) == RMIDPAREN) {
        t = new TreeNode("VariMore", nullptr, "VN");

    }
    else if (getCurLex(pointer) == LMIDPAREN) {
        t = new TreeNode("VariMore", nullptr, "VN");
        t->addChild(matchToken(LMIDPAREN));
        t->addChild(exp());
        t->addChild(matchToken(RMIDPAREN));
    }
    else if (getCurLex(pointer) == DOT) {
        t = new TreeNode("VariMore", nullptr, "VN");
        t->addChild(matchToken(DOT));
        t->addChild(fieldVar());
    }
    else { grammarerror(getCurLine(pointer), "VariMore",enumToString(getCurLex(pointer))); }
    return t;
}

//96. FieldVar -> ID FieldVarMore
TreeNode* fieldVar() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ID) {
        t = new TreeNode("FieldVar", nullptr, "VN");
        t->addChild(matchToken(ID));
        t->addChild(fieldVarMore());
    }
    else { grammarerror(getCurLine(pointer), "FieldVar",enumToString(getCurLex(pointer))); }
    return t;
}

//97. FieldVarMore -> ε | [ Exp ]
TreeNode* fieldVarMore() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == ASSIGN || getCurLex(pointer) == TIMES || getCurLex(pointer) == OVER ||
        getCurLex(pointer) == PLUS || getCurLex(pointer) == MINUS || getCurLex(pointer) == LT ||
        getCurLex(pointer) == EQ || getCurLex(pointer) == THEN || getCurLex(pointer) == ELSE ||
        getCurLex(pointer) == FI || getCurLex(pointer) == DO || getCurLex(pointer) == ENDWH ||
        getCurLex(pointer) == RPAREN || getCurLex(pointer) == END1 || getCurLex(pointer) == SEMI ||
        getCurLex(pointer) == COMMA) {
        t = new TreeNode("FieldVarMore", nullptr, "VN");
    }
    else if (getCurLex(pointer) == LMIDPAREN) {
        t = new TreeNode("FieldVarMore", nullptr, "VN");
        t->addChild(matchToken(LMIDPAREN));
        t->addChild(exp());
        t->addChild(matchToken(RMIDPAREN));
    }
    else { grammarerror(getCurLine(pointer), "FieldVarMore",enumToString(getCurLex(pointer))); }
    return t;
}


//99. CmpOp -> < | =
TreeNode* cmpOp() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == LT) {
        t = new TreeNode("CmpOP", nullptr, "VN");
        t->addChild(matchToken(LT));
    }
    else if (getCurLex(pointer) == EQ) {
        t = new TreeNode("CmpOP", nullptr, "VN");
        t->addChild(matchToken(EQ));
    }
    else { grammarerror(getCurLine(pointer), "CmpOP",enumToString(getCurLex(pointer))); }
    return t;
}
//101. AddOp -> + | -
TreeNode* addOp() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == PLUS) {
        t = new TreeNode("AddOP", nullptr, "VN");
        t->addChild(matchToken(PLUS));
    }
    else if (getCurLex(pointer) == MINUS) {
        t = new TreeNode("AddOP", nullptr, "VN");
        t->addChild(matchToken(MINUS));
    }
    else { grammarerror(getCurLine(pointer), "AddOP",enumToString(getCurLex(pointer))); }
    return t;
}

//103. MultOp -> * | /
TreeNode* multOp() {
    TreeNode* t = nullptr;
    if (getCurLex(pointer) == TIMES) {
        t = new TreeNode("MultOP", nullptr, "VN");
        t->addChild(matchToken(TIMES));
    }
    else if (getCurLex(pointer) == OVER) {
        t = new TreeNode("MultOP", nullptr, "VN");
        t->addChild(matchToken(OVER));
    }
    else { grammarerror(getCurLine(pointer), "MultOP",enumToString(getCurLex(pointer))); }
    return t;
}