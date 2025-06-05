#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iomanip>

class Assembler {
public:
    std::vector<uint32_t> assemble(const std::vector<std::string>& asmLines);

private:
    static std::string trim(const std::string& s);
    static uint8_t parseReg(const std::string& r);
};
