#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "Register.h"
#include "Immdiate.h"
#include "Enums.h"

// 内存操作数（用于load/store指令）
struct MemoryOperand {
    Register baseReg;       // 基址寄存器
    Register indexReg;      // 索引寄存器（可选）
    Immediate offset;       // 偏移量
    bool hasIndex;          // 是否有索引寄存器
    bool preIndex;          // 是否为前索引 [Rn, #imm]!
    bool postIndex;         // 是否为后索引 [Rn], #imm
    
    MemoryOperand(Register base, Immediate off = Immediate(0))
        : baseReg(base), offset(off), hasIndex(false), 
          preIndex(false), postIndex(false) {}
          
    MemoryOperand(Register base, Register index)
        : baseReg(base), indexReg(index), hasIndex(true),
          preIndex(false), postIndex(false) {}
};

struct DataProcInfo {
    DataProcOp operation;
    Register rd;
    Register rn;
    Register rm;
    Immediate imm;
    bool updateFlags;
    uint8_t shift;
};

struct MemoryInfo {
    MemoryOp operation;
    Register rt;
    MemoryOperand address;
    uint8_t size;
};

struct BranchInfo {
    BranchCondition condition;
    Register target;
    Immediate offset;
    bool isLink;
};

struct CompareInfo {
    Register rn;
    Register rm;
    Immediate imm;
    bool useImmediate;
};

struct MoveInfo {
    Register rd;
    Register rn;
    Immediate imm;
    bool useImmediate;
    uint8_t shift;
};

struct MulDivInfo {
    Register rd;
    Register rn;
    Register rm;
    Register ra;
    bool isSigned;
    bool hasAccumulate;
};

struct SystemInfo {
    SystemOp operation;
};

// 指令格式结构体
struct InstructionFormat {
    InstructionType type;
    
    // 使用 variant 来存储不同类型指令的特定数据
    std::variant<
        DataProcInfo,
        MemoryInfo,
        BranchInfo,
        CompareInfo,
        MoveInfo,
        MulDivInfo,
        SystemInfo
    > details;

    bool is32BitOp() const {
        return std::visit([](const auto& info) -> bool {
            using T = std::decay_t<decltype(info)>;
            if constexpr (std::is_same_v<T, DataProcInfo>) {
                return info.rd.is32Bit();
            } else if constexpr (std::is_same_v<T, MemoryInfo>) {
                return info.rt.is32Bit();
            } else if constexpr (std::is_same_v<T, MoveInfo>) {
                return info.rd.is32Bit();
            } else if constexpr (std::is_same_v<T, MulDivInfo>) {
                return info.rd.is32Bit();
            }
            return false;
        }, details);
    }
    
    bool is64BitOp() const { return !is32BitOp(); }
    
    std::string toString() const {
        // TODO
        return "InstructionFormat::toString()";
    }
};

// ========================== 指令构建器辅助类 ==========================

class InstructionBuilder {
public:
    // 数据处理指令构建器
    static InstructionFormat buildDataProcReg(
        DataProcOp op, Register rd, Register rn, Register rm, 
        bool updateFlags = false, uint8_t shift = 0) {
        
        InstructionFormat instr;
        instr.type = InstructionType::DATA_PROCESSING_REG;
        instr.details = DataProcInfo{
            op, rd, rn, rm, Immediate(), updateFlags, shift
        };
        return instr;
    }
    
    static InstructionFormat buildDataProcImm(
        DataProcOp op, Register rd, Register rn, Immediate imm,
        bool updateFlags = false) {
        
        InstructionFormat instr;
        instr.type = InstructionType::DATA_PROCESSING_IMM;
        instr.details = DataProcInfo{
            op, rd, rn, Register(), imm, updateFlags, 0
        };
        return instr;
    }
    
    // 内存访问指令构建器
    static InstructionFormat buildLoadStore(
        MemoryOp op, Register rt, MemoryOperand addr) {
        
        InstructionFormat instr;
        instr.type = InstructionType::LOAD_STORE;
        
        uint8_t size = 8; // 默认64位
        if (rt.is32Bit()) size = 4;
        
        switch (op) {
            case MemoryOp::LOAD_BYTE:
            case MemoryOp::STORE_BYTE:
                size = 1; break;
            case MemoryOp::LOAD_HALF:
            case MemoryOp::STORE_HALF:
                size = 2; break;
            case MemoryOp::LOAD_WORD:
            case MemoryOp::STORE_WORD:
                size = 4; break;
        }
        
        instr.details = MemoryInfo{op, rt, addr, size};
        return instr;
    }
    
    // 分支指令构建器
    static InstructionFormat buildBranch(
        Immediate offset, BranchCondition cond = BranchCondition::AL,
        bool isLink = false) {
        
        InstructionFormat instr;
        instr.type = (cond == BranchCondition::AL) ? 
            InstructionType::BRANCH_UNCOND : InstructionType::BRANCH_COND;
        
        instr.details = BranchInfo{
            cond, Register(), offset, isLink
        };
        return instr;
    }

    static InstructionFormat buildBranchReg(
        Register rd, bool isLink = false) {
        
        InstructionFormat instr;
        instr.type = InstructionType::BRANCH_REG;
        
        instr.details = BranchInfo{
            BranchCondition::AL, rd, NULL, isLink, 
        };
        return instr;
    }
    
    // 比较指令构建器
    static InstructionFormat buildCompare(Register rn, Register rm) {
        InstructionFormat instr;
        instr.type = InstructionType::COMPARE;
        instr.details = CompareInfo{
            rn, rm, Immediate(), false
        };
        return instr;
    }
    
    static InstructionFormat buildCompareImm(Register rn, Immediate imm) {
        InstructionFormat instr;
        instr.type = InstructionType::COMPARE;
        instr.details = CompareInfo{
            rn, Register(), imm, true
        };
        return instr;
    }
    
    // 移动指令构建器
    static InstructionFormat buildMoveReg(Register rd, Register rn) {
        InstructionFormat instr;
        instr.type = InstructionType::MOVE_REG;
        instr.details = MoveInfo{
            rd, rn, Immediate(), false, 0
        };
        return instr;
    }
    
    static InstructionFormat buildMoveImm(Register rd, Immediate imm) {
        InstructionFormat instr;
        instr.type = InstructionType::MOVE_IMM;
        instr.details = MoveInfo{
            rd, Register(), imm, true, 0
        };
        return instr;
    }
    
    static InstructionFormat buildSystem(SystemOp op) {
        InstructionFormat instr;
        instr.type = InstructionType::SYSTEM;
        instr.details = SystemInfo{op};
        return instr;
    }
};
