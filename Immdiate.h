#pragma once

#include <cstdint>

// 立即数类型
struct Immediate {
    int64_t value;      // 立即数值
    uint8_t bits;       // 有效位数 (8, 12, 16, 26等)
    bool isSigned;      // 是否有符号
    
    Immediate(int64_t val = 0, uint8_t b = 16, bool sign = true)
        : value(val), bits(b), isSigned(sign) {}
        
    // 获取符号扩展后的值
    int64_t getSignExtended() const {
        if (!isSigned) return value;
        
        int64_t mask = (1LL << bits) - 1;
        int64_t signBit = 1LL << (bits - 1);
        
        if (value & signBit) {
            return value | (~mask);  // 符号扩展
        }
        return value & mask;
    }
};
