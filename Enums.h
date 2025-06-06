#pragma once

// 分支条件
enum class BranchCondition {
    EQ = 0b0000,  // Equal (Z=1)
    NE = 0b0001,  // Not equal (Z=0)
    CS = 0b0010,  // Carry set (C=1)
    CC = 0b0011,  // Carry clear (C=0)
    MI = 0b0100,  // Minus (N=1)
    PL = 0b0101,  // Plus (N=0)
    VS = 0b0110,  // Overflow set (V=1)
    VC = 0b0111,  // Overflow clear (V=0)
    HI = 0b1000,  // Higher (C=1 && Z=0)
    LS = 0b1001,  // Lower or same (C=0 || Z=1)
    GE = 0b1010,  // Greater or equal (N==V)
    LT = 0b1011,  // Less than (N!=V)
    GT = 0b1100,  // Greater than (Z=0 && N==V)
    LE = 0b1101,  // Less than or equal (Z=1 || N!=V)
    AL = 0b1110,  // Always
    NV = 0b1111   // Never (reserved)
};

// 指令类型枚举
enum class InstructionType {
    // 数据处理指令
    DATA_PROCESSING_REG,     // ADD X1, X2, X3
    DATA_PROCESSING_IMM,     // ADD X1, X2, #100
    
    // 内存访问指令
    LOAD_STORE,              // LDR X1, [X2, #8]
    
    // 分支指令
    BRANCH_UNCOND,           // B label
    BRANCH_COND,             // B.EQ label
    BRANCH_LINK,             // BL label
    BRANCH_REG,              // BR X1, BLR X1
    
    // 比较指令
    COMPARE,                 // CMP X1, X2
    
    // 移动指令
    MOVE_REG,                // MOV X1, X2
    MOVE_IMM,                // MOV X1, #100
    MOVE_WIDE,               // MOVZ, MOVN, MOVK
    
    // 系统指令
    SYSTEM,                  // NOP, HLT, RET
    
    // 乘除指令
    MULTIPLY,                // MUL, MADD, MSUB
    DIVIDE                   // UDIV, SDIV
};

// 数据处理操作类型
enum class DataProcOp {
    // 算术运算
    ADD, ADDS, SUB, SUBS,
    ADC, ADCS, SBC, SBCS,
    
    // 逻辑运算
    AND, ANDS, ORR, EOR,
    BIC, ORN, EON,
    
    // 位移运算
    LSL, LSR, ASR, ROR
};

// 内存操作类型
enum class MemoryOp {
    LOAD_BYTE,      // LDRB
    LOAD_HALF,      // LDRH  
    LOAD_WORD,      // LDR (32-bit)
    LOAD_DWORD,     // LDR (64-bit)
    STORE_BYTE,     // STRB
    STORE_HALF,     // STRH
    STORE_WORD,     // STR (32-bit)
    STORE_DWORD     // STR (64-bit)
};
