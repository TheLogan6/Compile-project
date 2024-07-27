#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <setjmp.h>
#include <sstream>
#include <stdexcept>

//词法分析的相关错误-------------------------
// 标识符错误： 在DFA中只有 字符常量定义
class IllegalIdentifier : public std::exception{
public:
    int lineNo;
    std::string ch;
public:
    IllegalIdentifier(int x, std::string y):lineNo(x), ch(y){}
    const char* what() const noexcept override {
        std::stringstream ss;
        std::cout << "Illegal Identifier in line " << lineNo << ", Wrong Identifier:" << std::endl;
        return "Illegal Identifier";
    }
};

// 未知名字符错误
class UnknownCharacter : public std::exception{
public:
    int lineNo;
public:
    UnknownCharacter(int x):lineNo(x){}
    const char* what() const noexcept override {
        std::cout << "Unknown Character in line " << lineNo  << std::endl;
        return  "Unknown Character";
    }
};



// 语法分析------------------------------------
class GrammarError : public std::exception {
public:
    int lineNo;
    std::string origin;
    std::string now;
public:
    GrammarError(int lineNo, std::string& origin, std::string& now)
            : lineNo(lineNo), origin(origin), now(now) {}

    const char* what() const noexcept override {
        std::cout << "GrammarError in line " << lineNo << " : here should be " << origin << " instead of " << now << std::endl;
        return "Grammar Error";
    }
};


/**语义方向的错误*/
/*
 * 1. 重复定义
 * 2. 未定义标识符（类型和变量需要区分吗）
 * 3. 左值错误
 * 4. 右值错误
 * 5. 赋值语句两边类型不匹配
 * 6. 条件判断语句两边类型不匹配
 * 7. 函数调用参数类型不匹配
 * 8. 函数调用参数数目不对
 * 9. 函数调用有误
 * 10. 数组定义不符合规范
 * 11. 数组索引越界
 */

class ReDefine : public std::exception {//重定义
public:
    std::string name;
    int lineno;
public:
    ReDefine(std::string n,int l):name(n),lineno(l){}

    const char* what() const noexcept override {
        std::cout << name << " is ReDefined in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class UndefinedSymbol : public std::exception {//未定义
public:
    std::string name;
    int lineno;
public:
    UndefinedSymbol(std::string n,int l):name(n),lineno(l){}

    const char* what() const noexcept override {
        std::cout << name << " is UndefinedSymbol in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class LAssignInvalid : public std::exception {
public:
    std::string name;
    int lineno;
    std::string origin;
public:
    LAssignInvalid(std::string n,int l, std::string ori):name(n),lineno(l),origin(ori){}

    const char* what() const noexcept override {
        std::cout << name << " is " << origin << " , invalid left assign value in line " << lineno <<std::endl;
        return "Semantic Error";
    }
};

class RAssignInvalid : public std::exception {
public:
    std::string name;
    int lineno;
public:
    RAssignInvalid(std::string n,int l):name(n),lineno(l){}

    const char* what() const noexcept override {
        std::cout << name << " is a invalid right assign value in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class AssignMisMatch : public std::exception { //赋值语句类型不符
public:
    std::string left;
    std::string right;
    int lineno;
public:
    AssignMisMatch(std::string l, std::string r,int line):left(l),right(r),lineno(line){}

    const char* what() const noexcept override {
        std::cout << left << " do not match " << right << " in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class CompareMisMatch : public std::exception { //条件判断类型不符
public:
    std::string left;
    std::string right;
    int lineno;
public:
    CompareMisMatch(std::string l, std::string r,int line):left(l),right(r),lineno(line){}

    const char* what() const noexcept override {
        std::cout << left << " do not match " << right << " in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class ProcedureParamMisMatch : public std::exception { //过程调用参数不匹配
public:
    std::string left;
    std::string right;
    int lineno;
public:
    ProcedureParamMisMatch(std::string l, std::string r,int line):left(l),right(r),lineno(line){}

    const char* what() const noexcept override {
        std::cout << left << " do not match " << right << " in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class ProcedureCallError : public std::exception { //过程调用有误
public:
    std::string name;
    int lineno;
public:
    ProcedureCallError(std::string n,int l):name(n),lineno(l){}

    const char* what() const noexcept override {
        std::cout << name << " is not procedure in line " << lineno << std::endl;
        return "Semantic Error";
    }
};

class ArgumentNumMismatch : public std::exception { //实参数量不匹配
public:
    std::string name;
    int lineno;
    int correctnum;
public:
    ArgumentNumMismatch(std::string n,int l,int cn):name(n),lineno(l),correctnum(cn){}

    const char* what() const noexcept override {
        std::cout << "Procedure " << name << " mismatch the correct argument numbers in " << lineno \
        << ", it should be " <<  correctnum << "numbers" <<  std::endl;
        return "Semantic Error";
    }
};

class ArrayDefineError : public std::exception { //数组定义不符合规范
public:
    int lineno;
public:
    ArrayDefineError(int l):lineno(l){}

    const char* what() const noexcept override {
        std::cout  << "Array defined error in line " << lineno << std::endl;
        return "Semantic Error";
    }
};
class ArrayIndexMisMatch : public std::exception {
public:
    std::string name;
    int lineno;
public:
    ArrayIndexMisMatch(std::string n,int l):name(n),lineno(l){}

    const char* what() const noexcept override {
        std::cout  << "Array " << name << " has error index in line " << lineno << std::endl;
        return "Semantic Error";
    }
};
class ArrayIndexOutBound : public std::exception {
public:
    std::string name;
    int lineno;
    int shouldl, shouldr;
public:
    ArrayIndexOutBound(std::string n,int l, int sl, int sr):name(n),lineno(l),shouldl(sl),shouldr(sr){}

    const char* what() const noexcept override {
        std::cout  << "Array " << name << " index out of bound in line " << lineno  \
         << ", it should in range from " << shouldl << " to " << shouldr << std::endl;
        return "Semantic Error";
    }
};
