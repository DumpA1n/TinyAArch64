#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iomanip>

struct TokenInfo {
    std::string token;
    bool isValid = false;  // 是否合法token
    bool isReg = false;    // 是否寄存器
    bool isX = false;      // 是否X寄存器（否则默认W寄存器）
    bool isImm = false;    // 是否立即数
    bool isHex = false;    // 是否16进制立即数
};

class Assembler {
public:
    std::vector<uint32_t> assemble(const std::vector<std::string>& asmLines);
    std::vector<uint32_t> assemble(const std::string& asmLines);

private:
    std::vector<TokenInfo> parseTokens(const std::vector<std::string>& tokens);

    static std::string trim(const std::string& s);
    static uint8_t parseReg(const std::string& r);
};
