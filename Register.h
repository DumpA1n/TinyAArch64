#pragma once

#include <cstdint>
#include <string>

enum class RegWidth {
    W, // 32-bit
    X  // 64-bit
};

struct Register {
    uint8_t number;    // 寄存器编号 (0-30)
    RegWidth width;    // 寄存器位宽
    
    Register(uint8_t num = 0, RegWidth w = RegWidth::X) 
        : number(num), width(w) {}
    
    bool is32Bit() const { return width == RegWidth::W; }
    bool is64Bit() const { return width == RegWidth::X; }
    
    std::string toString() const {
        return (width == RegWidth::W ? "W" : "X") + std::to_string(number);
    }
};
