#include "CPU.h"

// ====================== 取指阶段 ======================
void CPU::fetch() {
    // 从内存读取指令（小端序）
    IR = 0;
    for (int i = 0; i < 4; i++) {
        IR |= (static_cast<uint32_t>(memory[PC + i]) << (i * 8));
    }
    
    // PC+4（指向下一条指令）
    PC += 4;
}

// ====================== 译码阶段 ======================
InstructionFormat CPU::decode() const {
    InstructionFormat instr;
    uint8_t opcode = (IR >> 26) & 0x3F; // 高6位是操作码
    
    switch (static_cast<Opcode>(opcode)) {
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_SDIV:
    case OP_UDIV:
    case OP_AND:
    case OP_ORR:
    case OP_EOR:
    case OP_MOV:
        instr.type = InstructionType::R_TYPE;
        instr.opcode = opcode;
        instr.rd = (IR >> 21) & 0x1F;
        instr.rn = (IR >> 16) & 0x1F;
        instr.rm = IR & 0x1F;
        break;
        
    case OP_ADDI:
    case OP_SUBI:
    case OP_ANDI:
    case OP_ORRI:
    case OP_EORI:
    case OP_MOVI:
    case OP_CMP:
        instr.type = InstructionType::I_TYPE;
        instr.opcode = opcode;
        instr.rd = (IR >> 21) & 0x1F;
        instr.rn = (IR >> 16) & 0x1F;
        instr.immediate = IR & 0xFFFF;
        break;
        
    case OP_LDR:
    case OP_STR:
        instr.type = InstructionType::D_TYPE;
        instr.opcode = opcode;
        instr.rt = (IR >> 21) & 0x1F;
        instr.rn = (IR >> 16) & 0x1F;
        instr.offset = IR & 0xFFFF; // 有符号偏移量
        // 符号扩展
        if (instr.offset & 0x8000) {
            instr.offset |= 0xFFFF0000;
        }
        break;
        
    case OP_B:
        instr.type = InstructionType::B_TYPE;
        instr.opcode = opcode;
        instr.offset = IR & 0x03FFFFFF; // 26位偏移量
        // 符号扩展
        if (instr.offset & 0x02000000) {
            instr.offset |= 0xFC000000;
        }
        break;
        
    case OP_B_COND:
        instr.type = InstructionType::B_TYPE;
        instr.opcode = opcode;
        instr.condition = (IR >> 22) & 0x0F;
        instr.offset = IR & 0x03FFFFFF; // 26位偏移量
        // 符号扩展
        if (instr.offset & 0x02000000) {
            instr.offset |= 0xFC000000;
        }
        break;
        
    case OP_RET:
        instr.type = InstructionType::R_TYPE;
        instr.opcode = opcode;
        break;
        
    case OP_HLT:
        instr.type = InstructionType::R_TYPE;
        instr.opcode = opcode;
        break;
        
    default:
        throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }
    
    return instr;
}

// ====================== 执行阶段 ======================
void CPU::execute(const InstructionFormat& instr) {
    switch (static_cast<Opcode>(instr.opcode)) {
    case OP_ADD:
        aluOperation(ALUOp::ADD, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_ADDI:
        aluOperation(ALUOp::ADD, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_SUB:
        aluOperation(ALUOp::SUB, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_SUBI:
        aluOperation(ALUOp::SUB, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_MUL:
        aluOperation(ALUOp::MUL, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_SDIV:
        aluOperation(ALUOp::SDIV, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_UDIV:
        aluOperation(ALUOp::UDIV, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_AND:
        aluOperation(ALUOp::AND, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_ANDI:
        aluOperation(ALUOp::AND, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_ORR:
        aluOperation(ALUOp::ORR, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_ORRI:
        aluOperation(ALUOp::ORR, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_EOR:
        aluOperation(ALUOp::EOR, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_EORI:
        aluOperation(ALUOp::EOR, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_MOV:
        regs[instr.rd] = regs[instr.rn];
        break;
        
    case OP_MOVI:
        regs[instr.rd] = instr.immediate;
        break;
        
    case OP_LDR: {
        uint64_t addr = regs[instr.rn] + instr.offset;
        regs[instr.rt] = readMemory<uint64_t>(addr);
        break;
    }
        
    case OP_STR: {
        uint64_t addr = regs[instr.rn] + instr.offset;
        writeMemory(addr, regs[instr.rt]);
        break;
    }
        
    case OP_B:
        // PC当前指向下一条指令，所以需要减去4
        // PC = PC - 4 + (instr.offset << 2);
        PC = PC + (instr.offset << 2);
        break;
        
    case OP_B_COND:
        if (checkCondition(instr.condition)) {
            // PC = PC - 4 + (instr.offset << 2);
            PC = PC + (instr.offset << 2);
        }
        break;
        
    case OP_BL:
        regs[30] = PC;
        PC = PC + (instr.offset << 2);
        break;
        
    case OP_BLR:
        regs[30] = PC;
        PC = regs[instr.rd];
        break;
        
    case OP_CMP:
        aluOperation(ALUOp::CMP, 0, regs[instr.rd], regs[instr.rn]);
        break;
        
    case OP_NOP:
        break;

    case OP_RET:
        PC = regs[30];
        break;
        
    case OP_HLT:
        throw std::runtime_error("HLT instruction executed");
        
    default:
        throw std::runtime_error("Unimplemented opcode: " + std::to_string(instr.opcode));
    }
}

// ====================== ALU操作 ======================
void CPU::aluOperation(ALUOp op, uint8_t rd, uint64_t a, uint64_t b, bool is32bit) {
    uint64_t result = 0;
    bool carry = false;
    bool overflow = false;
    
    if (is32bit) {
        // 32位操作：截断输入为32位
        int32_t a32 = static_cast<int32_t>(a & 0xFFFFFFFF);
        int32_t b32 = static_cast<int32_t>(b & 0xFFFFFFFF);
        int32_t result32 = 0;
        
        switch (op) {
        case ALUOp::ADD:
            result32 = a32 + b32;
            carry = (static_cast<uint32_t>(result32) < static_cast<uint32_t>(a32));
            overflow = ((a32 ^ result32) & (b32 ^ result32)) >> 31;
            break;
            
        case ALUOp::SUB:
            result32 = a32 - b32;
            carry = (static_cast<uint32_t>(a32) >= static_cast<uint32_t>(b32));
            overflow = ((a32 ^ b32) & (a32 ^ result32)) >> 31;
            break;
            
        case ALUOp::MUL:
            result32 = a32 * b32;
            break;
            
        case ALUOp::SDIV:
            if (b32 == 0) throw std::runtime_error("Division by zero");
            result32 = a32 / b32;
            break;
            
        case ALUOp::UDIV:
            if (b32 == 0) throw std::runtime_error("Division by zero");
            result32 = static_cast<uint32_t>(a32) / static_cast<uint32_t>(b32);
            break;
            
        case ALUOp::AND:
            result32 = a32 & b32;
            break;
            
        case ALUOp::ORR:
            result32 = a32 | b32;
            break;
            
        case ALUOp::EOR:
            result32 = a32 ^ b32;
            break;
            
        case ALUOp::CMP:
            result32 = a32 - b32;
            statusReg.N = (result32 >> 31) & 1;
            statusReg.Z = (result32 == 0);
            statusReg.C = (static_cast<uint32_t>(a32) >= static_cast<uint32_t>(b32));
            statusReg.V = ((a32 ^ b32) & (a32 ^ result32)) >> 31;
            return;
            
        default:
            throw std::runtime_error("Unsupported ALU operation");
        }
        
        result = static_cast<uint64_t>(static_cast<uint32_t>(result32));
        
        // 更新状态寄存器（基于32位结果）
        statusReg.N = (result32 >> 31) & 1;
        statusReg.Z = (result32 == 0);
        statusReg.C = carry;
        statusReg.V = overflow;
        
    } else {
        // 64位操作
        int64_t a64 = static_cast<int64_t>(a);
        int64_t b64 = static_cast<int64_t>(b);
        int64_t result64 = 0;
        
        switch (op) {
        case ALUOp::ADD:
            result64 = a64 + b64;
            carry = (static_cast<uint64_t>(result64) < static_cast<uint64_t>(a64));
            overflow = ((a64 ^ result64) & (b64 ^ result64)) >> 63;
            break;
            
        case ALUOp::SUB:
            result64 = a64 - b64;
            carry = (static_cast<uint64_t>(a64) >= static_cast<uint64_t>(b64));
            overflow = ((a64 ^ b64) & (a64 ^ result64)) >> 63;
            break;
            
        case ALUOp::MUL:
            result64 = a64 * b64;
            break;
            
        case ALUOp::SDIV:
            if (b64 == 0) throw std::runtime_error("Division by zero");
            result64 = a64 / b64;
            break;
            
        case ALUOp::UDIV:
            if (b64 == 0) throw std::runtime_error("Division by zero");
            result64 = static_cast<uint64_t>(a64) / static_cast<uint64_t>(b64);
            break;
            
        case ALUOp::AND:
            result64 = a64 & b64;
            break;
            
        case ALUOp::ORR:
            result64 = a64 | b64;
            break;
            
        case ALUOp::EOR:
            result64 = a64 ^ b64;
            break;
            
        case ALUOp::CMP:
            result64 = a64 - b64;
            statusReg.N = (result64 >> 63) & 1;
            statusReg.Z = (result64 == 0);
            statusReg.C = (static_cast<uint64_t>(a64) >= static_cast<uint64_t>(b64));
            statusReg.V = ((a64 ^ b64) & (a64 ^ result64)) >> 63;
            return;
            
        default:
            throw std::runtime_error("Unsupported ALU operation");
        }
        
        result = static_cast<uint64_t>(result64);
        
        // 更新状态寄存器（基于64位结果）
        statusReg.N = (result64 >> 63) & 1;
        statusReg.Z = (result64 == 0);
        statusReg.C = carry;
        statusReg.V = overflow;
    }
    
    // 写回结果到寄存器
    if (rd < NUM_REGS) {
        regs[rd] = result;
    }
}

// ====================== 条件检查 ======================
bool CPU::checkCondition(uint8_t condition) const {
    switch (static_cast<ConditionCode>(condition)) {
    case COND_EQ: return statusReg.Z; // Z set
    case COND_NE: return !statusReg.Z; // Z clear
    case COND_LT: return statusReg.N != statusReg.V; // N != V
    case COND_GE: return statusReg.N == statusReg.V; // N == V
    case COND_GT: return !statusReg.Z && (statusReg.N == statusReg.V); // Z clear and N == V
    case COND_LE: return statusReg.Z || (statusReg.N != statusReg.V); // Z set or N != V
    case COND_AL: return true; // Always
    default: return false;
    }
}
