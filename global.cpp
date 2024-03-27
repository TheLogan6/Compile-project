
#include "global.h"
#include "lexical.h"
#include "grammar.h"




Token tokenList[1010];// tokenList

int Index;
string ch1;
string ch2;
string ch3;

TreeNode* treeroot;
TreeNode* root;

int LL1Tbl[104][104]; // LL1

extern TreeNode* currentTree;

extern Token* currentToken;

extern int strline;

extern string outstr[512];

extern int is_newLL1_correct;


