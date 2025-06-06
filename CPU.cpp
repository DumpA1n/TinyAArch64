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
    bool is32Bits = false;
    
    switch (static_cast<Opcode>(opcode)) {
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_SDIV:
    case OP_UDIV:
    case OP_AND:
    case OP_ORR:
    case OP_EOR: {
        Register rt{(IR >> 21) & 0x1F, RegWidth::X};
        Register rn{(IR >> 16) & 0x1F, RegWidth::X};
        Register rm{IR & 0x1F, RegWidth::X};
        instr = InstructionBuilder::buildDataProcReg(convertToDataProcOp(opcode), rt, rn, rm);
        break;
    }
        
    case OP_ADDI:
    case OP_SUBI:
    case OP_ANDI:
    case OP_ORRI:
    case OP_EORI: {
        Register rt{(IR >> 21) & 0x1F, RegWidth::W};
        Register rn{(IR >> 16) & 0x1F, RegWidth::W};
        Immediate imm{IR & 0xFFFF};
        instr = InstructionBuilder::buildDataProcImm(convertToDataProcOp(opcode), rt, rn, imm);
        break;
    }

    case OP_MOV: {
        Register rt{(IR >> 21) & 0x1F, RegWidth::X};
        Register rn{(IR >> 16) & 0x1F, RegWidth::X};
        instr = InstructionBuilder::buildMoveReg(rt, rn);
        break;
    }
    case OP_MOVI: {
        Register rt{(IR >> 21) & 0x1F, RegWidth::W};
        Immediate imm{IR & 0xFFFF};
        instr = InstructionBuilder::buildMoveImm(rt, imm);
        break;
    }

    case OP_CMP: 
    case OP_CMPI: 

    case OP_LDRB:
    case OP_LDRH:
    case OP_LDRW:
    case OP_LDRD:
    case OP_STRB:
    case OP_STRH:
    case OP_STRW:
    case OP_STRD: {
        Register rt{(IR >> 21) & 0x1F, RegWidth::X};
        Register rn{(IR >> 16) & 0x1F, RegWidth::X};
        Immediate offset{IR & 0xFFFF}; // 有符号偏移量
        // 符号扩展
        if (offset.value & 0x8000) {
            offset.value |= 0xFFFF0000;
        }
        instr = InstructionBuilder::buildLoadStore(convertToMemoryOp(opcode), rt, MemoryOperand{rn, offset});
        break;
    }
        
    // case OP_B:
    //     instr.type = InstructionType::BRANCH_UNCOND;
    //     instr.opcode = opcode;
    //     instr.offset = IR & 0x03FFFFFF; // 26位偏移量
    //     // 符号扩展
    //     if (instr.offset & 0x02000000) {
    //         instr.offset |= 0xFC000000;
    //     }
    //     break;
        
    // case OP_B_COND:
    //     instr.type = InstructionType::BRANCH_COND;
    //     instr.opcode = opcode;
    //     instr.condition = (IR >> 22) & 0x0F;
    //     instr.offset = IR & 0x03FFFFFF; // 26位偏移量
    //     // 符号扩展
    //     if (instr.offset & 0x02000000) {
    //         instr.offset |= 0xFC000000;
    //     }
    //     break;
        
    case OP_NOP:
    case OP_RET:
    case OP_HLT:
        instr = InstructionBuilder::buildSystem(convertToSystemOp(opcode));
        break;
        
    default:
        throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }
    
    return instr;
}

// ====================== 执行阶段 ======================
void CPU::execute(const InstructionFormat& instr) {
    switch (instr.type) {
        case InstructionType::DATA_PROCESSING_REG:
        case InstructionType::DATA_PROCESSING_IMM:
            executeDataProcessing(instr);
            break;
            
        case InstructionType::LOAD_STORE:
            executeLoadStore(instr);
            break;
            
        case InstructionType::BRANCH_UNCOND:
        case InstructionType::BRANCH_COND:
        case InstructionType::BRANCH_LINK:
        case InstructionType::BRANCH_REG:
            executeBranch(instr);
            break;
            
        case InstructionType::COMPARE:
            executeCompare(instr);
            break;
            
        case InstructionType::MOVE_REG:
        case InstructionType::MOVE_IMM:
            executeMove(instr);
            break;
            
        case InstructionType::MULTIPLY:
        case InstructionType::DIVIDE:
            executeMultiplyDivide(instr);
            break;
            
        case InstructionType::SYSTEM:
            executeSystem(instr);
            break;
            
        default:
            throw std::runtime_error("Unknown instruction type");
    }
}

// 数据处理指令执行
void CPU::executeDataProcessing(const InstructionFormat& instr) {
    auto& info = std::get<DataProcInfo>(instr.details);
    
    // 获取操作数
    uint64_t operand1 = getRegisterValue(info.rn);
    uint64_t operand2;
    
    if (instr.type == InstructionType::DATA_PROCESSING_IMM) {
        operand2 = info.imm.getSignExtended();
    } else {
        operand2 = getRegisterValue(info.rm);
    }
    
    // 应用移位（如果有）
    if (info.shift > 0 && instr.type == InstructionType::DATA_PROCESSING_REG) {
        operand2 = operand2 << info.shift;
    }
    
    // 执行ALU操作
    ALUOp aluOp = convertToALUOp(info.operation);
    bool is32bit = info.rd.is32Bit();
    
    aluOperation(aluOp, info.rd.number, operand1, operand2, is32bit);
}

// 内存访问指令执行
void CPU::executeLoadStore(const InstructionFormat& instr) {
    auto& info = std::get<MemoryInfo>(instr.details);
    
    // 计算内存地址
    uint64_t baseAddr = getRegisterValue(info.address.baseReg);
    uint64_t address = baseAddr + info.address.offset.getSignExtended();
    
    if (info.address.hasIndex) {
        address += getRegisterValue(info.address.indexReg);
    }
    
    // 执行内存操作
    switch (info.operation) {
        case MemoryOp::LOAD_BYTE:
            setRegisterValue(info.rt, readMemory<uint8_t>(address));
            break;
        case MemoryOp::LOAD_HALF:
            setRegisterValue(info.rt, readMemory<uint16_t>(address));
            break;
        case MemoryOp::LOAD_WORD:
            setRegisterValue(info.rt, readMemory<uint32_t>(address));
            break;
        case MemoryOp::LOAD_DWORD:
            setRegisterValue(info.rt, readMemory<uint64_t>(address));
            break;
        case MemoryOp::STORE_BYTE:
            writeMemory<uint8_t>(address, static_cast<uint8_t>(getRegisterValue(info.rt)));
            break;
        case MemoryOp::STORE_HALF:
            writeMemory<uint16_t>(address, static_cast<uint16_t>(getRegisterValue(info.rt)));
            break;
        case MemoryOp::STORE_WORD:
            writeMemory<uint32_t>(address, static_cast<uint32_t>(getRegisterValue(info.rt)));
            break;
        case MemoryOp::STORE_DWORD:
            writeMemory<uint64_t>(address, static_cast<uint64_t>(getRegisterValue(info.rt)));
            break;
    }
    
    // 处理前/后索引模式
    if (info.address.preIndex || info.address.postIndex) {
        uint64_t newBaseAddr = info.address.postIndex ? 
            address : baseAddr + info.address.offset.getSignExtended();
        setRegisterValue(info.address.baseReg, newBaseAddr);
    }
}

// 分支指令执行
void CPU::executeBranch(const InstructionFormat& instr) {
    auto& info = std::get<BranchInfo>(instr.details);
    
    bool shouldBranch = true;
    
    if (instr.type == InstructionType::BRANCH_COND) {
        shouldBranch = checkCondition(info.condition);
    }
    
    if (shouldBranch) {
        if (info.isLink) {
            regs[30] = PC;  // 保存返回地址到LR (X30)
        }
        
        if (instr.type == InstructionType::BRANCH_REG) {
            PC = getRegisterValue(info.target);
        } else {
            PC = PC + (info.offset.getSignExtended() << 2);
        }
    }
}

// 比较指令执行
void CPU::executeCompare(const InstructionFormat& instr) {
    auto& info = std::get<CompareInfo>(instr.details);
    
    uint64_t operand1 = getRegisterValue(info.rn);
    uint64_t operand2 = info.useImmediate ? 
        info.imm.getSignExtended() : getRegisterValue(info.rm);
    
    // 比较操作不写回结果，只更新标志位
    bool is32bit = info.rn.is32Bit();
    aluOperation(ALUOp::CMP, 0, operand1, operand2, is32bit);
}

// 移动指令执行
void CPU::executeMove(const InstructionFormat& instr) {
    auto& info = std::get<MoveInfo>(instr.details);
    
    uint64_t value;
    if (info.useImmediate) {
        value = info.imm.getSignExtended();
    } else {
        value = getRegisterValue(info.rn);
    }
    
    setRegisterValue(info.rd, value);
}

// 乘除指令执行
void CPU::executeMultiplyDivide(const InstructionFormat& instr) {
    auto& info = std::get<MulDivInfo>(instr.details);
    
    uint64_t operand1 = getRegisterValue(info.rn);
    uint64_t operand2 = getRegisterValue(info.rm);
    
    ALUOp op = info.isSigned ? ALUOp::SDIV : ALUOp::UDIV;
    if (instr.type == InstructionType::MULTIPLY) {
        op = ALUOp::MUL;
    }
    
    bool is32bit = info.rd.is32Bit();
    aluOperation(op, info.rd.number, operand1, operand2, is32bit);
}

// 系统指令执行
void CPU::executeSystem(const InstructionFormat& instr) {
    auto& info = std::get<SystemInfo>(instr.details);
    
    switch (info.operation) {
        case SystemOp::NOP: // NOP
            break;
        case SystemOp::RET: // RET
            PC = regs[30];
            break;
        case SystemOp::HLT: // HLT
            throw std::runtime_error("HLT instruction executed");
        default:
            throw std::runtime_error("Unknown system operation");
    }
}

// ====================== 数据转换 ======================
ALUOp CPU::convertToALUOp(DataProcOp op) const {
    switch (op) {
        case DataProcOp::ADD:
        case DataProcOp::ADDS:
            return ALUOp::ADD;
        case DataProcOp::SUB:
        case DataProcOp::SUBS:
            return ALUOp::SUB;
        case DataProcOp::AND:
        case DataProcOp::ANDS:
            return ALUOp::AND;
        case DataProcOp::ORR:
            return ALUOp::ORR;
        case DataProcOp::EOR:
            return ALUOp::EOR;
        default:
            throw std::runtime_error("Unsupported data processing operation");
    }
}

DataProcOp CPU::convertToDataProcOp(uint8_t opcode) const {
    switch (static_cast<Opcode>(opcode)) {
        case OP_ADD:
        case OP_ADDI:
            return DataProcOp::ADD;
        case OP_SUB:
        case OP_SUBI:
            return DataProcOp::SUB;
        case OP_AND:
        case OP_ANDI:
            return DataProcOp::AND;
        case OP_ORR:
        case OP_ORRI:
            return DataProcOp::ORR;
        case OP_EOR:
        case OP_EORI:
            return DataProcOp::EOR;

        // case OP_LSL:
        //     return DataProcOp::LSL;
        // case OP_LSR:
        //     return DataProcOp::LSR;
        // case OP_ASR:
        //     return DataProcOp::ASR;
        // case OP_ROR:
        //     return DataProcOp::ROR;

        default:
            throw std::runtime_error("Unsupported opcode for DataProcOp");
    }
}

MemoryOp CPU::convertToMemoryOp(uint8_t opcode) const {
    switch (static_cast<Opcode>(opcode)) {
        case OP_LDRB: return MemoryOp::LOAD_BYTE;
        case OP_LDRH: return MemoryOp::LOAD_HALF;
        case OP_LDRW: return MemoryOp::LOAD_WORD;
        case OP_LDRD: return MemoryOp::LOAD_DWORD;
        case OP_STRB: return MemoryOp::STORE_BYTE;
        case OP_STRH: return MemoryOp::STORE_HALF;
        case OP_STRW: return MemoryOp::STORE_WORD;
        case OP_STRD: return MemoryOp::STORE_DWORD;

        default:
            throw std::runtime_error("Unsupported opcode for DataProcOp");
    }
}

SystemOp CPU::convertToSystemOp(uint8_t opcode) const {
    switch (static_cast<Opcode>(opcode)) {
        case OP_NOP: return SystemOp::NOP;
        case OP_RET: return SystemOp::RET;
        case OP_HLT: return SystemOp::HLT;

        default:
            throw std::runtime_error("Unsupported opcode for DataProcOp");
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
bool CPU::checkCondition(BranchCondition condition) const {
    switch (condition) {
        case BranchCondition::EQ: return statusReg.Z;
        case BranchCondition::NE: return !statusReg.Z;
        case BranchCondition::CS: return statusReg.C;
        case BranchCondition::CC: return !statusReg.C;
        case BranchCondition::MI: return statusReg.N;
        case BranchCondition::PL: return !statusReg.N;
        case BranchCondition::VS: return statusReg.V;
        case BranchCondition::VC: return !statusReg.V;
        case BranchCondition::HI: return statusReg.C && !statusReg.Z;
        case BranchCondition::LS: return !statusReg.C || statusReg.Z;
        case BranchCondition::GE: return statusReg.N == statusReg.V;
        case BranchCondition::LT: return statusReg.N != statusReg.V;
        case BranchCondition::GT: return !statusReg.Z && (statusReg.N == statusReg.V);
        case BranchCondition::LE: return statusReg.Z || (statusReg.N != statusReg.V);
        case BranchCondition::AL: return true;
        case BranchCondition::NV: return false;
        default: return false;
    }
}

// ====================== 寄存器操作 ======================
uint64_t CPU::getXReg(uint8_t reg) const {
    if (reg >= NUM_REGS) throw std::runtime_error("Invalid register number");
    return regs[reg];
}

void CPU::setXReg(uint8_t reg, uint64_t value) {
    if (reg >= NUM_REGS) throw std::runtime_error("Invalid register number");
    regs[reg] = value;
}

uint32_t CPU::getWReg(uint8_t reg) const {
    if (reg >= NUM_REGS) throw std::runtime_error("Invalid register number");
    return static_cast<uint32_t>(regs[reg] & 0xFFFFFFFF);
}

void CPU::setWReg(uint8_t reg, uint32_t value) {
    if (reg >= NUM_REGS) throw std::runtime_error("Invalid register number");
    // 写入W寄存器时，高32位清零
    regs[reg] = static_cast<uint32_t>(value & 0xFFFFFFFF);
}

uint64_t CPU::getRegisterValue(const Register& reg) {
    if (reg.is32Bit()) {
        return getWReg(reg.number);
    } else {
        return getXReg(reg.number);
    }
}

void CPU::setRegisterValue(const Register& reg, uint64_t value) {
    if (reg.is32Bit()) {
        setWReg(reg.number, static_cast<uint32_t>(value));
    } else {
        setXReg(reg.number, value);
    }
}
