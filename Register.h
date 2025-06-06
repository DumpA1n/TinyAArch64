#pragma once

#include <cstdint>
#include <string>

enum class RegWidth {
    W, // 32-bit
    X  // 64-bit
};

// 通用寄存器 (X0-X30)
struct Register {
    uint8_t number;    // 寄存器编号
    RegWidth width;    // 寄存器位宽
    
    Register(uint8_t num = 0, RegWidth w = RegWidth::X) 
        : number(num), width(w) {}
    
    inline bool is32Bit() const { return width == RegWidth::W; }
    inline bool is64Bit() const { return width == RegWidth::X; }
    
    inline std::string toString() const {
        return (width == RegWidth::W ? "W" : "X") + std::to_string(number);
    }
};

// 状态寄存器 (NZCV)
struct StatusRegister {
    bool N; // Negative
    bool Z; // Zero
    bool C; // Carry
    bool V; // Overflow
    
    inline void reset() { N = Z = C = V = false; }
    
    inline std::string toString() const {
        return std::string("N=") + (N ? "1" : "0") + 
                          " Z="  + (Z ? "1" : "0") +
                          " C="  + (C ? "1" : "0") +
                          " V="  + (V ? "1" : "0");
    }
};
