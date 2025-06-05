#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <stdexcept>
#include <array>

#include "Assembler.h"

// ========================== 指令系统设计 ==========================
// 指令格式定义
enum class InstructionType { R_TYPE, I_TYPE, D_TYPE, B_TYPE };

struct InstructionFormat {
    InstructionType type;
    uint8_t opcode;
    uint8_t rd;    // R型和I型的目标寄存器
    uint8_t rn;    // R型、I型、D型的第一个源寄存器
    uint8_t rm;    // R型的第二个源寄存器
    uint8_t rt;    // D型的目标寄存器
    uint16_t immediate; // I型的立即数
    uint8_t condition; // B型的条件码
    int8_t offset;   // D型和B型的偏移量
};

// 指令操作码定义
enum Opcode {
    OP_ADD = 0b000000,
    OP_ADDI = 0b000001,
    OP_SUB = 0b000010,
    OP_SUBI = 0b000011,
    OP_AND = 0b000100,
    OP_ANDI = 0b000101,
    OP_ORR = 0b000110,
    OP_ORRI = 0b000111,
    OP_EOR = 0b001000,
    OP_EORI = 0b001001,
    OP_MOV = 0b001010,
    OP_MOVI = 0b001011,
    OP_LDR = 0b001100,
    OP_STR = 0b001101,
    OP_B = 0b001110,
    OP_B_COND = 0b001111,
    OP_CMP = 0b010000,
    OP_MUL = 0b010001,
    OP_SDIV = 0b010010,
    OP_UDIV = 0b010011,
    OP_BL = 0b010100,
    OP_BLR = 0b010101,
    OP_NOP = 0b111101,
    OP_RET = 0b111110,
    OP_HLT = 0b111111
};

// 条件码定义
enum ConditionCode {
    COND_EQ = 0b0000,  // Equal (Z=1)
    COND_NE = 0b0001,  // Not equal (Z=0)
    COND_LT = 0b0010,  // Signed less than (N!=V)
    COND_GE = 0b0011,  // Signed greater than or equal (N=V)
    COND_GT = 0b0100,  // Signed greater than (Z=0 and N=V)
    COND_LE = 0b0101,  // Signed less than or equal (Z=1 or N!=V)
    COND_AL = 0b1111   // Always (unconditional)
};

// ========================== 数据通路组件 ==========================
// 状态寄存器 (NZCV)
struct StatusRegister {
    bool N; // Negative
    bool Z; // Zero
    bool C; // Carry
    bool V; // Overflow
    
    void reset() { N = Z = C = V = false; }
    
    std::string toString() const {
        return std::string("N=") + (N ? "1" : "0") + 
                          " Z="  + (Z ? "1" : "0") +
                          " C="  + (C ? "1" : "0") +
                          " V="  + (V ? "1" : "0");
    }
};

// ALU操作定义
enum class ALUOp {
    ADD, SUB, MUL, SDIV, UDIV, AND, ORR, EOR, NOT, LSL, LSR, ASR, CMP, PASS_A, PASS_B
};

// 模拟的CPU类
class CPU {
public:
    static const int NUM_REGS = 32;   // 31个通用寄存器
    static const int MEM_SIZE = 1024; // 1KB内存
    
    CPU() : memory(MEM_SIZE, 0), PC(0), IR(0), statusReg{} {
        reset();
    }

    void reset() {
        PC = 0;
        IR = 0;
        statusReg.reset();
        std::fill(regs.begin(), regs.end(), 0);
        std::fill(memory.begin(), memory.end(), 0);
    }

    // 加载程序到内存
    void loadProgram(const std::vector<uint32_t>& program) {
        if (program.size() * 4 > MEM_SIZE) {
            throw std::runtime_error("Program too large for memory");
        }
        
        for (size_t i = 0; i < program.size(); i++) {
            uint32_t word = program[i];
            memory[i*4] = word & 0xFF;
            memory[i*4+1] = (word >> 8) & 0xFF;
            memory[i*4+2] = (word >> 16) & 0xFF;
            memory[i*4+3] = (word >> 24) & 0xFF;
        }
    }

    // 执行一个指令周期
    void step() {
        // 1. 取指
        fetch();
        
        // 2. 译码
        InstructionFormat instr = decode();
        
        // 3. 执行
        execute(instr);
    }

    // 打印当前状态
    void printState() const {
        std::cout << "===== CPU State =====" << std::endl;
        std::cout << "PC: 0x" << std::hex << std::setw(8) << std::setfill('0') << PC << std::dec << std::endl;
        std::cout << "IR: 0x" << std::hex << std::setw(8) << std::setfill('0') << IR << std::dec << std::endl;
        std::cout << "Status: " << statusReg.toString() << std::endl;
        
        std::cout << "Registers:" << std::endl;
        for (int i = 0; i < NUM_REGS; i++) {
            std::cout << "R" << i << ": 0x" << std::hex << std::setw(16) << std::setfill('0') 
                 << regs[i] << std::dec;
            if (i % 4 == 3) std::cout << std::endl;
            else std::cout << "\t";
        }
        std::cout << std::endl;
        
        // // 打印内存前64字节
        // std::cout << "Memory (first 64 bytes):" << std::endl;
        // for (int i = 0; i < 64; i++) {
        //     if (i % 16 == 0) {
        //         if (i > 0) std::cout << std::endl;
        //         std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << i << ": ";
        //     }
        //     std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(memory[i]) << " ";
        // }
        // std::cout << std::dec << std::endl << std::endl;
    }

private:
    std::vector<uint8_t> memory;     // 字节寻址内存
    std::array<uint64_t, NUM_REGS> regs; // 寄存器文件
    uint64_t PC;                // 程序计数器
    uint32_t IR;                // 指令寄存器
    StatusRegister statusReg;   // 状态寄存器

    // ====================== 取指阶段 ======================
    void fetch();

    // ====================== 译码阶段 ======================
    InstructionFormat decode() const;

    // ====================== 执行阶段 ======================
    void execute(const InstructionFormat& instr);

    // ====================== ALU操作 ======================
    void aluOperation(ALUOp op, uint8_t rd, int64_t a, int64_t b);

    // ====================== 条件检查 ======================
    bool checkCondition(uint8_t condition) const;

    // ====================== 内存访问 ======================
    template<typename T>
    T readMemory(uint64_t address) const {
        constexpr size_t size = sizeof(T);

        if (address + size > MEM_SIZE) {
            throw std::runtime_error("Memory read out of bounds: " + std::to_string(address));
        }
        
        T value = 0;
        for (size_t i = 0; i < size; ++i) {
            value |= static_cast<uint32_t>(memory[address + i]) << (i * 8);
        }
        return value;
    }

    template<typename T>
    void writeMemory(uint64_t address, T value) {
        constexpr size_t size = sizeof(T);

        if (address + size > MEM_SIZE) {
            throw std::runtime_error("Memory write out of bounds: " + std::to_string(address));
        }
        
        for (size_t i = 0; i < size; ++i) {
            memory[address + i] = (value >> (i * 8)) & 0xFF;
        }
    }
};
