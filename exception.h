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
        //转换为C风格的必要操作
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
class SyntaxError : public std::exception {
public:
    int lineNo;
    std::string origin;
    std::string now;
public:
    SyntaxError(int lineNo, std::string& origin, std::string& now)
            : lineNo(lineNo), origin(origin), now(now) {}

    const char* what() const noexcept override {
        std::cout << "SyntaxError in line " << lineNo << " : here should be " << origin << " instead of " << now << std::endl;
        return "Syntax Error";
    }
};

class RedundancyError : public std::exception {
public:
    int lineNo;

public:
    RedundancyError(int lineNo) : lineNo(lineNo) {}

    const char* what() const noexcept override {
        std::stringstream ss;
        ss << "RedundancyError in line " << lineNo << " : superfluous code after end_program";
        std::string errorMsg = ss.str();
        return errorMsg.c_str();
    }
};