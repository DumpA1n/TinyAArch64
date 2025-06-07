#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <stdexcept>
#include <array>
#include <utility>

#include "Assembler.h"
#include "Instruction.h"
#include "Enums.h"

// ========================== 数据通路组件 ==========================

// 模拟的CPU类
class CPU {
public:
    static const int NUM_REGS = 32;   // 31个通用寄存器
    static const int MEM_SIZE = 1048576; // 1MB内存
    
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
    uint64_t SP;                // 栈
    StatusRegister statusReg;   // 状态寄存器

    // ====================== 取指阶段 ======================
    void fetch();

    // ====================== 译码阶段 ======================
    InstructionFormat decode() const;

    // ====================== 执行阶段 ======================
    void execute(const InstructionFormat& instr);

    // ====================== 指令执行 ======================
    void executeDataProcessing(const InstructionFormat& instr);
    void executeLoadStore(const InstructionFormat& instr);
    void executeBranch(const InstructionFormat& instr);
    void executeCompare(const InstructionFormat& instr);
    void executeMove(const InstructionFormat& instr);
    void executeMultiplyDivide(const InstructionFormat& instr);
    void executeSystem(const InstructionFormat& instr);

    // ====================== 数据转换 ======================
    ALUOp convertToALUOp(DataProcOp op) const;
    DataProcOp convertToDataProcOp(uint8_t opcode) const;
    MemoryOp convertToMemoryOp(uint8_t opcode) const;
    SystemOp convertToSystemOp(uint8_t opcode) const;

    // ====================== ALU操作 ======================
    void aluOperation(ALUOp op, uint8_t rd, uint64_t a, uint64_t b, bool is32bit = false);

    // ====================== 条件检查 ======================
    bool checkCondition(BranchCondition condition) const;

    // ====================== 寄存器操作 ======================
    inline uint64_t getXReg(uint8_t reg) const;
    inline void     setXReg(uint8_t reg, uint64_t value);
    inline uint32_t getWReg(uint8_t reg) const;
    inline void     setWReg(uint8_t reg, uint32_t value);
    inline uint64_t getRegisterValue(const Register& reg);
    inline void     setRegisterValue(const Register& reg, uint64_t value);

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
