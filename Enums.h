#pragma once

// ALU操作定义
enum class ALUOp {
    ADD, SUB, MUL, SDIV, UDIV, AND, ORR, EOR, NOT, LSL, LSR, ASR, CMP
};

// 指令操作码
enum Opcode {
    OP_ADD     = 0b000000,
    OP_ADDI    = 0b000001,
    OP_SUB     = 0b000010,
    OP_SUBI    = 0b000011,

    OP_AND     = 0b000100,
    OP_ANDI    = 0b000101,
    OP_ORR     = 0b000110,
    OP_ORRI    = 0b000111,
    OP_EOR     = 0b001000,
    OP_EORI    = 0b001001,

    OP_MOV     = 0b001010,
    OP_MOVI    = 0b001011,

    OP_CMP     = 0b001100,
    OP_CMPI    = 0b001101,

    OP_MUL     = 0b001110,
    OP_SDIV    = 0b001111,
    OP_UDIV    = 0b010000,

    OP_LDRB    = 0b010001,
    OP_LDRH    = 0b010010,
    OP_LDRW    = 0b010011,
    OP_LDRD    = 0b010100,

    OP_STRB    = 0b010101,
    OP_STRH    = 0b010110,
    OP_STRW    = 0b010111,
    OP_STRD    = 0b011000,

    OP_B       = 0b011001,
    OP_B_COND  = 0b011010,
    OP_BL      = 0b011011,
    OP_BLR     = 0b011100,
    OP_BR      = 0b011101,

    OP_RET     = 0b011110,
    OP_HLT     = 0b011111,
    OP_NOP     = 0b111111
};

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
    MUL, SDIV, UDIV,
    
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

// 系统操作类型
enum class SystemOp {
    NOP,
    RET, 
    HLT
};
