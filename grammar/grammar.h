#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <setjmp.h>
#include <sstream>
#include <stdexcept>
#include "lexical.h"
#include "global.h"

using namespace std;

//语法节点
class TreeNode
{
public:
    Token* tk;
    vector<TreeNode*> child;
    vector<TreeNode*> sibiling;
    string ifTermimal;
    string name;          //节点表示的终极符、非终极符

    TreeNode(string nm, Token* t = nullptr, string ter = "VN")
    {
        name = nm;
        ifTermimal = ter;
        if(ter == "VN")
        {
            tk = nullptr;
        }
        else if(ter == "VT")  //终极符
        {
            tk = t;
        }
        child.clear();
        sibiling.clear();
        child.resize(0);
        sibiling.resize(0);
    }

    void addChild(TreeNode* t) {
        child.push_back(t);
    }

    bool ifNoChild(){
        return child.size() == 0;
    }
};

class GrammarTree{
public:

    int nodeNum;
    TreeNode* root;
    TreeNode* now;

    GrammarTree(int x = 0, TreeNode* ro = nullptr, TreeNode* no = nullptr):nodeNum(x),root(ro),now(no){}

    TreeNode* goRoot(){
        return this->root;
    }
    TreeNode* getNowNode(){
        return this->now;
    }
};

extern TreeNode* matchToken(LexType lex);

extern void dfsPrintTree(TreeNode* p,int level);
extern void printToPy(TreeNode* p, int level);

//1. Program -> ProgramHead DeclarePart ProgramBody .
extern TreeNode* program();

//2. ProgramHead -> PROGRAM ProgramName
extern TreeNode* programHead();

//3. ProgramName -> ID
extern TreeNode* programName();

//4.DeclarePart -> TypeDec VarDec ProcDec
extern TreeNode* declarePart();

//6. TypeDec	::=	ε	| TypeDeclaration
extern TreeNode* typeDec();

//7. TypeDeclaration -> ε | TypeDeclaration
extern TreeNode* typeDeclaration();

//8. TypeDecList -> TypeId = TypeName ; TypeDecMore
extern TreeNode* typeDecList();

//9. TypeDecMore -> ε | TypeDecList
extern TreeNode* typeDecMore();

//11. TypeId -> ID
extern TreeNode* typeID();

//12 TypeName -> BaseType | StructureType | ID
extern TreeNode* typeName();

//15. BaseType -> INTEGER | CHAR
extern TreeNode* baseType();

//17.StructureType -> ArrayType | RecType
extern TreeNode* structureType();

//19.ArrayType -> ARRAY [ Low .. Top ] OF BaseType
extern TreeNode* arrayType();

//20.Low -> INTC
extern TreeNode* low();

//21.Top -> INTC
extern TreeNode* top();

//22.RecType -> RECORD FieldDecList END
extern TreeNode* recType();

//23.FieldDecList -> BaseType IdList ; FieldDecMore | ArrayType IdList ; FieldDecMore
extern TreeNode* fieldDecList();

//25. FieldDecMore -> ε | FieldDecList
extern TreeNode* fieldDecMore();

//27.IdList -> ID IdMore
extern TreeNode* IDList();

//28.IdMore -> ε | , IdList
extern TreeNode* IDMore();

//30.VarDec -> ε | VarDeclaration
extern TreeNode* varDec();

//32. VarDeclaration -> VAR VarDecList
extern TreeNode* varDeclaration();

//33.VarDecList -> TypeName VarIdList ; VarDecMore
extern TreeNode* varDecList();

//34.VarDecMore -> ε | VarDecList
extern TreeNode* varDecMore();

//36.VarIdList -> ID VarIdMore
extern TreeNode* varIDList();

//37.VarIdMore -> ε | , VarIdList
extern TreeNode* varIDMore();

//39. ProcDec -> ε | ProcDeclaration
extern TreeNode* procDec();

//41.ProcDeclaration -> PROCEDURE ProcName ( ParamList ) ; ProcDecPart ProcBody ProcDecMore
extern TreeNode* procDeclaration();

//42.ProcDecMore ->	ε  | ProcDeclaration
//ProcDecMore 等价于  procDec 了

//44. ProcName -> ID
extern TreeNode* procName();

//45. ParamList -> ε | ParamDecList
extern TreeNode* paramList();

//47. ParamDecList -> Param ParamMore
extern TreeNode* paramDecList();

//48. ParamMore -> ε | ; ParamDecList
extern TreeNode* paramMore();

//50. Param -> TypeName FormList | VAR TypeName FormList
extern TreeNode* param();

//52. FormList -> ID FidMore
extern TreeNode* formList();

//53. FidMore -> ε | , FormList
extern TreeNode* fidMore();
//55. ProcDecPart -> DeclarePart
extern TreeNode* procDecPart();

//56. ProcBody -> ProgramBody
extern TreeNode* procBody();

//57. ProgramBody -> BEGIN StmList END
extern TreeNode* programBody();

//58. StmList -> Stm StmMore
extern TreeNode* stmList();

//59. StmMore -> ε | ; StmList
extern TreeNode* stmMore();

//61. Stm -> ConditionalStm | LoopStm | InputStm | OutputStm | ReturnStm | ID AssCall
extern TreeNode* stm();

//67. AssCall -> AssignmentRest | CallStmRest
extern TreeNode* assCall();

//69. AssignmentRest -> VariMore := Exp
extern TreeNode* assignmentRest();

//70. ConditionalStm -> IF RelExp THEN StmList ELSE StmList FI
extern TreeNode* conditionalStm();

//71. LoopStm -> WHILE RelExp DO StmList ENDWH
extern TreeNode* loopStm();

//72. InputStm -> READ ( Invar )
extern TreeNode* inputStm();

//73. Invar -> ID
extern TreeNode* inVar();

//74. OutputStm -> WRITE ( Exp )
extern TreeNode* outputStm();

//75. ReturnStm -> RETURN ( Exp )
extern TreeNode* returnStm();

//76. CallStmRest -> ( ActParamList )
extern TreeNode* callStmRest();

//77. ActParamList -> ε | Exp ActParamMore
extern TreeNode* actparamList();

//79. ActParamMore -> ε | , ActParamList
extern TreeNode* actparamMore();
//
//81. RelExp -> Exp OtherRelE
extern TreeNode* relExp();

//82. OtherRelE -> CmpOp Exp
extern TreeNode* otherRelE();

//83. Exp -> Term OtherTerm
extern TreeNode* exp();

//84. OtherTerm -> ε | AddOp Exp
extern TreeNode* otherTerm();

//86. Term -> Factor OtherFactor
extern TreeNode* term();

//87. OtherFactor  ->  ε    |  MultOp  Term
extern TreeNode* otherFactor();

//89. Factor -> ( Exp ) | INTC | Variable
extern TreeNode* factor();

//92. Variable -> ID VariMore
extern TreeNode* variable();

//93. VariMore -> ε | [ Exp ] | . FieldVar
extern TreeNode* variMore();

//96. FieldVar -> ID FieldVarMore
extern TreeNode* fieldVar();

//97. FieldVarMore -> ε | [ Exp ]
extern TreeNode* fieldVarMore();

//99. CmpOp -> < | =
extern TreeNode* cmpOp();

//101. AddOp -> + | -
extern TreeNode* addOp();

//103. MultOp -> * | /
extern TreeNode* multOp();












