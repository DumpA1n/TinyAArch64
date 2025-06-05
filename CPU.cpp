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
    case OP_AND:
    case OP_OR:
    case OP_XOR:
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
    case OP_ORI:
    case OP_XORI:
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
        
    case OP_AND:
        aluOperation(ALUOp::AND, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_ANDI:
        aluOperation(ALUOp::AND, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_OR:
        aluOperation(ALUOp::OR, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_ORI:
        aluOperation(ALUOp::OR, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_XOR:
        aluOperation(ALUOp::XOR, instr.rd, regs[instr.rn], regs[instr.rm]);
        break;
        
    case OP_XORI:
        aluOperation(ALUOp::XOR, instr.rd, regs[instr.rn], instr.immediate);
        break;
        
    case OP_MOV:
        regs[instr.rd] = regs[instr.rn];
        break;
        
    case OP_MOVI:
        regs[instr.rd] = instr.immediate;
        break;
        
    case OP_LDR: {
        uint32_t addr = regs[instr.rn] + instr.offset;
        regs[instr.rt] = readMemory(addr);
        break;
    }
        
    case OP_STR: {
        uint32_t addr = regs[instr.rn] + instr.offset;
        writeMemory(addr, regs[instr.rt]);
        break;
    }
        
    case OP_B:
        // PC当前指向下一条指令，所以需要减去4
        PC = PC - 4 + (instr.offset << 2);
        break;
        
    case OP_B_COND:
        if (checkCondition(instr.condition)) {
            PC = PC - 4 + (instr.offset << 2);
        }
        break;
        
    case OP_CMP:
        aluOperation(ALUOp::CMP, 0, regs[instr.rn], regs[instr.rd]);
        break;
        
    case OP_HLT:
        throw std::runtime_error("HLT instruction executed");
        
    default:
        throw std::runtime_error("Unimplemented opcode: " + std::to_string(instr.opcode));
    }
}

// ====================== ALU操作 ======================
void CPU::aluOperation(ALUOp op, uint8_t rd, uint32_t a, uint32_t b) {
    uint32_t result = 0;
    bool carry = false;
    bool overflow = false;
    
    switch (op) {
    case ALUOp::ADD:
        result = a + b;
        // 进位标志：无符号溢出
        carry = (result < a);
        // 溢出标志：有符号溢出
        overflow = ((a ^ result) & (b ^ result)) >> 31;
        break;
        
    case ALUOp::SUB:
        result = a - b;
        // 进位标志：无符号无借位
        carry = (a >= b);
        // 溢出标志：有符号溢出
        overflow = ((a ^ b) & (a ^ result)) >> 31;
        break;
        
    case ALUOp::AND:
        result = a & b;
        break;
        
    case ALUOp::OR:
        result = a | b;
        break;
        
    case ALUOp::XOR:
        result = a ^ b;
        break;
        
    case ALUOp::CMP:
        result = a - b;
        // 设置标志位但不存储结果
        statusReg.N = (result >> 31) & 1;
        statusReg.Z = (result == 0);
        statusReg.C = (a >= b); // 无借位
        statusReg.V = ((a ^ b) & (a ^ result)) >> 31;
        return; // 不写回寄存器
        
    default:
        throw std::runtime_error("Unsupported ALU operation");
    }
    
    // 更新状态寄存器
    statusReg.N = (result >> 31) & 1;
    statusReg.Z = (result == 0);
    statusReg.C = carry;
    statusReg.V = overflow;
    
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

// ====================== 内存访问 ======================
uint32_t CPU::readMemory(uint32_t address) const {
    if (address + 4 > MEM_SIZE) {
        throw std::runtime_error("Memory read out of bounds: " + std::to_string(address));
    }
    
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        value |= static_cast<uint32_t>(memory[address + i]) << (i * 8);
    }
    return value;
}

void CPU::writeMemory(uint32_t address, uint32_t value) {
    if (address + 4 > MEM_SIZE) {
        throw std::runtime_error("Memory write out of bounds: " + std::to_string(address));
    }
    
    for (int i = 0; i < 4; i++) {
        memory[address + i] = (value >> (i * 8)) & 0xFF;
    }
}
