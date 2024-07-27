#pragma GCC optimize("O0")
#include "codegen.h"
#include <fstream>
#include <cctype>
using namespace std;

extern Token tokenList[1010];
extern int pointer;
extern GrammarTree* tree;

//1-3为grammar和semantic展示，4-6为中间代码和目标代码展示，
#define INPUTDIR "C:\\Users\\86138\\Desktop\\MyComplierC\\presentation\\premips6.txt"

#define LEXOUTPUTFILEDIR "C:\\Users\\86138\\Desktop\\MyComplierC\\preoutput\\presem1lex.txt"
#define GRAOUTPUTFILEDIR "C:\\Users\\86138\\Desktop\\MyComplierC\\preoutput\\presem1gra.txt"
#define SEMANOUTPUTFILEDIR "C:\\Users\\86138\\Desktop\\MyComplierC\\preoutput\\presem1semantic.txt"
#define MIDCODEOUTPUTDIR "C:\\Users\\86138\\Desktop\\MyComplierC\\preoutput\\presem1mid.txt"
#define MIPSCODEOUTPUTDIR "C:\\Users\\86138\\Desktop\\MyComplierC\\preoutput\\presem1mipscode.txt"


#define LEXICALPRINT
#define GRAMMARMAIN
#define SEMANTICMAIN
#define MIDCODEMAIN
#define MIPSCODEMAIN




int main(){
    initReservedWord(); //初始化保留字
    FILE *fp;
    std::ofstream outputFile(MIPSCODEOUTPUTDIR, std::ios::out | std::ios::binary);
    if(outputFile.is_open()){
        outputFile << u8"\uFEFF"; // 写入 BOM (Byte Order Mark)，表示文件以 UTF-8 编码
    }

    fp = fopen(INPUTDIR, "r");
    fflush(stdout);
    lexicalAnalyse(fp);
#ifdef LEXICALPRINT
    freopen(LEXOUTPUTFILEDIR, "w", stdout);
    printTokenList();
    freopen(GRAOUTPUTFILEDIR, "w", stdout); // 输出重定向
#endif

    TreeNode* t = program();
    tree = new GrammarTree(0, t, t);

#ifdef GRAMMARMAIN
    freopen(GRAOUTPUTFILEDIR, "w", stdout); // 输出重定向
    cout << "Part2 : Grammar Analysis" << endl;  //奇怪的重定问题
    if (tokenList[pointer].wd.tok == ENDFILE) {
        cout << "grammar endfile !" << endl;
    }
//    tree->dfsPrintTree(t,0);
    tree->printToPy(t, 0);
    freopen(SEMANOUTPUTFILEDIR, "w", stdout); // 输出重定向
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
    midgen.beautPrintExp();

#endif
#ifdef MIPSCODEMAIN
    freopen(MIPSCODEOUTPUTDIR, "w", stdout);
    cout << "  ### Part5 : MIPS code generate!" << endl;  //奇怪的重定问题
    FinalCodeGen mipsgen(midgen.argExpList, analyzer.scope, midgen.actrecord,midgen.callorder);
    mipsgen.finalCodeGen();
#endif
    fclose(fp);
}
