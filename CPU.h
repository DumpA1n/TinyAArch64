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
#include "Log.h"

// ========================== 数据通路组件 ==========================

// 模拟的CPU类
class CPU {
public:
    static const int32_t NUM_REGS     = 32;       // 31个通用寄存器
    static const uint64_t MEM_SIZE    = 0x100000; // 1MB内存
    static const uint64_t STACK_BASE  = 0x100000; // 栈顶
    static const uint64_t STACK_LIMIT = 0x000800; // 栈底

private:
    CPU() : memory(MEM_SIZE, 0), PC(0), IR(0), statusReg{} {
        reset();
    }
    ~CPU() = default;

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;

public:
    static CPU& GetInstance() {
        static CPU instance;
        return instance;
    }

    void reset() {
        PC = 0;
        IR = 0;
        statusReg.reset();
        std::fill(regs.begin(), regs.end(), 0);
        std::fill(memory.begin(), memory.end(), 0);
        regs[31] = STACK_BASE; // X31作为SP寄存器
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

    // 打印状态
    void printState() const;
    void printRegisterState() const;
    void printRegisterState2() const;
    void printMemoryState(size_t n = 64) const;

    // 接口
    uint64_t getReg(uint8_t idx) const { return regs[idx]; }
    uint64_t getPC() const { return PC; }
    uint64_t getSP() const { return regs[31]; }
    uint64_t getIR() const { return IR; }
    StatusRegister getStatusReg() const { return statusReg; }

private:
    std::vector<uint8_t> memory;         // 虚拟内存
    std::array<uint64_t, NUM_REGS> regs; // 寄存器文件
    uint64_t PC;                         // 程序计数器
    uint32_t IR;                         // 指令寄存器
    StatusRegister statusReg;            // 状态寄存器


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
