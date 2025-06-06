#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "Register.h"
#include "Immdiate.h"
#include "Enums.h"

// 指令格式结构体
struct InstructionFormat {
    InstructionType type;
    uint8_t opcode;         // 原始操作码
    
    // 使用 variant 来存储不同类型指令的特定数据
    std::variant<
        struct {  // DATA_PROCESSING_REG/IMM
            DataProcOp operation;
            Register rd;        // 目标寄存器
            Register rn;        // 第一个操作数寄存器
            Register rm;        // 第二个操作数寄存器 (仅reg类型)
            Immediate imm;      // 立即数 (仅imm类型)
            bool updateFlags;   // 是否更新标志位 (ADDS vs ADD)
            uint8_t shift;      // 移位量
        } dataProcInfo,
        
        struct {  // LOAD_STORE
            MemoryOp operation;
            Register rt;        // 目标/源寄存器
            MemoryOperand address;
            uint8_t size;       // 访问大小 (1,2,4,8字节)
        } memoryInfo,
        
        struct {  // BRANCH_*
            BranchCondition condition;  // 分支条件
            Register target;            // 目标寄存器 (BR/BLR)
            Immediate offset;           // 分支偏移
            bool isLink;               // 是否保存返回地址
        } branchInfo,
        
        struct {  // COMPARE
            Register rn;        // 第一个比较寄存器
            Register rm;        // 第二个比较寄存器
            Immediate imm;      // 立即数比较值
            bool useImmediate;  // 是否使用立即数
        } compareInfo,
        
        struct {  // MOVE_*
            Register rd;        // 目标寄存器
            Register rn;        // 源寄存器
            Immediate imm;      // 立即数
            bool useImmediate;  // 是否使用立即数
            uint8_t shift;      // 移位量 (MOVK使用)
        } moveInfo,
        
        struct {  // MULTIPLY/DIVIDE
            Register rd;        // 目标寄存器
            Register rn;        // 第一个操作数
            Register rm;        // 第二个操作数
            Register ra;        // 累加寄存器 (MADD/MSUB)
            bool isSigned;      // 是否有符号运算
            bool hasAccumulate; // 是否有累加操作
        } mulDivInfo,
        
        struct {  // SYSTEM
            uint16_t sysOp;     // 系统操作码
        } systemInfo
        
    > details;
    
    // 便利方法
    bool is32BitOp() const {
        return std::visit([](const auto& info) -> bool {
            using T = std::decay_t<decltype(info)>;
            if constexpr (std::is_same_v<T, decltype(dataProcInfo)>) {
                return info.rd.is32Bit();
            } else if constexpr (std::is_same_v<T, decltype(memoryInfo)>) {
                return info.rt.is32Bit();
            } else if constexpr (std::is_same_v<T, decltype(moveInfo)>) {
                return info.rd.is32Bit();
            } else if constexpr (std::is_same_v<T, decltype(mulDivInfo)>) {
                return info.rd.is32Bit();
            }
            return false;
        }, details);
    }
    
    bool is64BitOp() const { return !is32BitOp(); }
    
    std::string toString() const {
        // 返回指令的字符串表示
        return "InstructionFormat::toString() - TODO";
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
        instr.details = decltype(instr.dataProcInfo){
            op, rd, rn, rm, Immediate(), updateFlags, shift
        };
        return instr;
    }
    
    static InstructionFormat buildDataProcImm(
        DataProcOp op, Register rd, Register rn, Immediate imm,
        bool updateFlags = false) {
        
        InstructionFormat instr;
        instr.type = InstructionType::DATA_PROCESSING_IMM;
        instr.details = decltype(instr.dataProcInfo){
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
        
        instr.details = decltype(instr.memoryInfo){op, rt, addr, size};
        return instr;
    }
    
    // 分支指令构建器
    static InstructionFormat buildBranch(
        Immediate offset, BranchCondition cond = BranchCondition::AL,
        bool isLink = false) {
        
        InstructionFormat instr;
        instr.type = (cond == BranchCondition::AL) ? 
            InstructionType::BRANCH_UNCOND : InstructionType::BRANCH_COND;
        
        instr.details = decltype(instr.branchInfo){
            cond, Register(), offset, isLink
        };
        return instr;
    }
    
    // 比较指令构建器
    static InstructionFormat buildCompare(Register rn, Register rm) {
        InstructionFormat instr;
        instr.type = InstructionType::COMPARE;
        instr.details = decltype(instr.compareInfo){rn, rm, Immediate(), false};
        return instr;
    }
    
    static InstructionFormat buildCompareImm(Register rn, Immediate imm) {
        InstructionFormat instr;
        instr.type = InstructionType::COMPARE;
        instr.details = decltype(instr.compareInfo){rn, Register(), imm, true};
        return instr;
    }
    
    // 移动指令构建器
    static InstructionFormat buildMoveReg(Register rd, Register rn) {
        InstructionFormat instr;
        instr.type = InstructionType::MOVE_REG;
        instr.details = decltype(instr.moveInfo){
            rd, rn, Immediate(), false, 0
        };
        return instr;
    }
    
    static InstructionFormat buildMoveImm(Register rd, Immediate imm) {
        InstructionFormat instr;
        instr.type = InstructionType::MOVE_IMM;
        instr.details = decltype(instr.moveInfo){
            rd, Register(), imm, true, 0
        };
        return instr;
    }
};
