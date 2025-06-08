#include "Assembler.h"

#include <unordered_set>

#include "CPU.h"

static std::unordered_map<std::string, std::pair<Opcode, Opcode>> OpcodeMap = {
    {"ADD", {OP_ADD, OP_ADDI}},
    {"SUB", {OP_SUB, OP_SUBI}},
    {"MUL", {OP_MUL, OP_MUL}},
    {"SDIV", {OP_SDIV, OP_SDIV}},
    {"UDIV", {OP_UDIV, OP_UDIV}},
    {"AND", {OP_AND, OP_ANDI}},
    {"ORR", {OP_ORR, OP_ORRI}},
    {"EOR", {OP_EOR, OP_EORI}},
    {"NOP", {OP_NOP, OP_NOP}},
    {"RET", {OP_RET, OP_RET}},
    {"HLT", {OP_HLT, OP_HLT}}
};

static std::unordered_map<std::string, enum class BranchCondition> B_COND_Map = {
    {"B.EQ", BranchCondition::EQ},
    {"B.NE", BranchCondition::NE},
    {"B.CS", BranchCondition::CS},
    {"B.CC", BranchCondition::CC},
    {"B.MI", BranchCondition::MI},
    {"B.PL", BranchCondition::PL},
    {"B.VS", BranchCondition::VS},
    {"B.VC", BranchCondition::VC},
    {"B.HI", BranchCondition::HI},
    {"B.LS", BranchCondition::LS},
    {"B.GE", BranchCondition::GE},
    {"B.LT", BranchCondition::LT},
    {"B.GT", BranchCondition::GT},
    {"B.LE", BranchCondition::LE},
    {"B.AL", BranchCondition::AL},
    {"B.NV", BranchCondition::NV},
    {"BEQ", BranchCondition::EQ},
    {"BNE", BranchCondition::NE},
    {"BCS", BranchCondition::CS},
    {"BCC", BranchCondition::CC},
    {"BMI", BranchCondition::MI},
    {"BPL", BranchCondition::PL},
    {"BVS", BranchCondition::VS},
    {"BVC", BranchCondition::VC},
    {"BHI", BranchCondition::HI},
    {"BLS", BranchCondition::LS},
    {"BGE", BranchCondition::GE},
    {"BLT", BranchCondition::LT},
    {"BGT", BranchCondition::GT},
    {"BLE", BranchCondition::LE},
    {"BAL", BranchCondition::AL},
    {"BNV", BranchCondition::NV}
};

std::vector<uint32_t> Assembler::assemble(const std::vector<std::string>& asmLines) {
    std::vector<uint32_t> machineCode;
    std::unordered_map<std::string, int> labelAddresses;
    std::vector<std::pair<int, std::string>> pendingLabels;

    int pc = 0;

    // 第一次遍历：处理标签
    for (const auto& line : asmLines) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.rfind("//", 0) == 0) continue;

        if (trimmed.back() == ':') {
            std::string label = trimmed.substr(0, trimmed.length() - 1);
            labelAddresses[label] = pc;
        } else {
            pc += 4;
        }
    }

    pc = 0;

    // 第二次遍历：汇编指令
    for (const auto& line : asmLines) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.back() == ':' || trimmed.rfind("//", 0) == 0) continue;
        std::cout << "trimmed instruction: " << trimmed << std::endl;

        std::istringstream iss(trimmed);
        std::string token;
        std::vector<std::string> tokens;

        while (iss >> token) {
            token.erase(std::remove_if(token.begin(), token.end(), [](char c) {
                return c == ',' || c == '[' || c == ']';
            }), token.end());
            tokens.push_back(token);
        }
        std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), 
                        [](unsigned char c) { return std::toupper(c); });

        if (tokens.empty()) throw std::runtime_error("None operands");

        std::string opcode = tokens[0];

        if (tokens[0] == ".INT" || tokens[0] == ".FLOAT") {
            if (tokens.size() < 2) throw std::runtime_error("缺少数值: " + trimmed);

            uint32_t value = 0;
            std::string valToken = tokens[1];
            if (valToken.find("0x") == 0 || valToken.find("0X") == 0)
                value = std::stoul(valToken, nullptr, 16);
            else
                value = std::stoul(valToken, nullptr, 10);

            machineCode.push_back(value);
        }
        else if (opcode == "MOV") {
            if (tokens.size() < 3) throw std::runtime_error("Too few operands: " + trimmed);
            auto parsed = parseTokens(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
            for (const auto& pt : parsed) { if (!pt.isValid) throw std::runtime_error("Instruction Invalid: " + trimmed); }
            uint8_t sf = parsed[0].isX ? 1 : 0;
            uint8_t rd_num = parseReg(parsed[0].token);
            uint8_t rn_num = 0;
            uint32_t imm_val = 0;
            uint32_t instr = (sf << 31) | ((parsed[1].isReg ? OP_MOV : OP_MOVI) << 26) | (rd_num << 21);
            if (parsed[1].isReg) {
                rn_num = parseReg(parsed[1].token);
                instr |= (rn_num << 16);
            } else if (parsed[1].isImm) {
                imm_val = std::stoi(parsed[1].token.substr(1), nullptr, parsed[1].isHex ? 16 : 10);
                instr |= (imm_val & 0xFFFF); // 截断低16位
            }
            machineCode.push_back(instr);
        }
        else if (opcode == "ADD" || opcode == "SUB" || opcode == "AND" || opcode == "ORR" || opcode == "EOR") {
            if (tokens.size() < 4) throw std::runtime_error("Too few operands: " + trimmed);
            auto parsed = parseTokens(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
            for (const auto& pt : parsed) { if (!pt.isValid) throw std::runtime_error("Instruction Invalid: " + trimmed); }
            uint8_t sf = parsed[0].isX ? 1 : 0;
            uint8_t rd_num = parseReg(parsed[0].token);
            uint8_t rn_num = parseReg(parsed[1].token);
            uint8_t rm_num = 0;
            uint32_t imm_val = 0;
            uint32_t instr = (sf << 31) | ((parsed[2].isReg ? OpcodeMap[opcode].first : OpcodeMap[opcode].second) << 26) | (rd_num << 21) | (rn_num << 16);
            if (parsed[2].isReg) {
                rm_num = parseReg(parsed[2].token);
                instr |= rm_num;
            } else if (parsed[2].isImm) {
                imm_val = std::stoi(parsed[2].token.substr(1), nullptr, parsed[2].isHex ? 16 : 10);
                instr |= (imm_val & 0xFFFF);
            }
            machineCode.push_back(instr);
        }
        else if (opcode == "MUL" || opcode == "SDIV" || opcode == "UDIV") {
            if (tokens.size() < 4) throw std::runtime_error("Too few operands: " + trimmed);
            auto parsed = parseTokens(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
            for (const auto& pt : parsed) { if (!pt.isValid) throw std::runtime_error("Instruction Invalid: " + trimmed); }
            uint8_t sf = parsed[0].isX ? 1 : 0;
            uint8_t rd_num = parseReg(parsed[0].token);
            uint8_t rn_num = parseReg(parsed[1].token);
            uint8_t rm_num = parseReg(parsed[2].token);
            uint32_t instr = (sf << 31) | (OpcodeMap[opcode].first << 26) | (rd_num << 21) | (rn_num << 16) | rm_num;
            machineCode.push_back(instr);
        }
        else if (opcode == "CMP") {
            if (tokens.size() < 3) throw std::runtime_error("Too few operands: " + trimmed);
            auto parsed = parseTokens(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
            for (const auto& pt : parsed) { if (!pt.isValid) throw std::runtime_error("Instruction Invalid: " + trimmed); }
            uint8_t sf = parsed[0].isX ? 1 : 0;
            uint8_t rd_num = parseReg(parsed[0].token);
            uint8_t rn_num = 0;
            uint32_t imm_val = 0;
            uint32_t instr = (sf << 31) | ((parsed[1].isReg ? OP_CMP : OP_CMPI) << 26) | (rd_num << 21);
            if (parsed[1].isReg) {
                rn_num = parseReg(parsed[1].token);
                instr |= (rn_num << 16);
            } else if (parsed[1].isImm) {
                imm_val = std::stoi(parsed[1].token.substr(1), nullptr, parsed[1].isHex ? 16 : 10);
                instr |= (imm_val & 0xFFFF);
            }
            machineCode.push_back(instr);
        }
        else if (opcode == "LDR" || opcode == "STR" || opcode == "LDRH" || opcode == "STRH" || opcode == "LDRB" || opcode == "STRB") {
            if (tokens.size() < 3) throw std::runtime_error("Too few operands: " + trimmed);

            auto parsed = parseTokens(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
            for (const auto& pt : parsed) {
                if (!pt.isValid) throw std::runtime_error("Instruction Invalid: " + trimmed);
            }

            uint8_t sf = parsed[0].isX ? 1 : 0;
            uint8_t rt_num = parseReg(parsed[0].token);
            uint8_t rn_num = parseReg(parsed[1].token);
            uint32_t imm_val = 0;

            if (parsed.size() > 2 && parsed[2].isImm) {
                imm_val = std::stoi(parsed[2].token.substr(1), nullptr, parsed[2].isHex ? 16 : 10);
            }

            uint8_t opcodeVal = 0;
            if (opcode == "LDR")  opcodeVal = parsed[0].isX ? OP_LDRD : OP_LDRW;
            else if (opcode == "STR")  opcodeVal = parsed[0].isX ? OP_STRD : OP_STRW;
            else if (opcode == "LDRH") opcodeVal = OP_LDRH;
            else if (opcode == "STRH") opcodeVal = OP_STRH;
            else if (opcode == "LDRB") opcodeVal = OP_LDRB;
            else if (opcode == "STRB") opcodeVal = OP_STRB;

            uint32_t instr = (sf << 31) | (opcodeVal << 26) | (rt_num << 21) | (rn_num << 16) | (imm_val & 0xFFFF);
            machineCode.push_back(instr);
        }
        else if (opcode == "B") {
            if (tokens.size() < 2) throw std::runtime_error("Too few operands: " + trimmed);
            std::string label = tokens[1];
            pendingLabels.push_back({pc, label});
            uint32_t instr = (OP_B << 26);
            machineCode.push_back(instr); // 占位，后续填充
        }
        else if (opcode == "BL") {
            if (tokens.size() < 2) throw std::runtime_error("Too few operands: " + trimmed);
            std::string label = tokens[1];
            pendingLabels.push_back({pc, label});
            uint32_t instr = (OP_BL << 26);
            machineCode.push_back(instr); // 占位，后续填充
        }
        else if (B_COND_Map.find(opcode) != B_COND_Map.end()) {
            if (tokens.size() < 2) throw std::runtime_error("Too few operands: " + trimmed);
            std::string label = tokens[1];
            pendingLabels.push_back({pc, label});
            uint8_t condition = static_cast<uint8_t>(B_COND_Map[opcode]);
            uint32_t instr = (OP_B_COND << 26) | ((condition & 0x0F) << 22);
            machineCode.push_back(instr); // 占位，后续填充
        }
        else if (opcode == "HLT" || opcode == "RET" || opcode == "NOP") {
            uint32_t instr = (OpcodeMap[opcode].first << 26);
            machineCode.push_back(instr);
        }
        else {
            std::cerr << "未知指令: " << tokens[0] << std::endl;
            exit(1);
        }

        pc += 4;
    }

    // 填补跳转地址
    for (auto& [addr, label] : pendingLabels) {
        if (labelAddresses.count(label)) {
            int labelAddr = labelAddresses[label];
            int8_t offset = (labelAddr - (addr + 4)) >> 2;
            machineCode[addr / 4] |= (offset & 0b1111111111111111111111);
        } else {
            std::cerr << "未知标签: " << label << std::endl;
            exit(1);
        }
    }

    for (const auto& code : machineCode) {
        for (int i = 31; i >= 0; --i) {
            std::cout << ((code >> i) & 1);
        }
        std::cout << std::endl;
    }

    return machineCode;
}

std::vector<uint32_t> Assembler::assemble(const std::string& asmLines) {
    std::istringstream iss(asmLines);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    return assemble(lines);
}

std::string Assembler::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

uint8_t Assembler::parseReg(const std::string& r) {
    if (r == "SP" || r == "sp") {
        return 31;
    }
    if (r[0] != 'W' && r[0] != 'w' && r[0] != 'X' && r[0] != 'x') {
        throw std::runtime_error("无效寄存器名: " + r);
    }
    return stoi(r.substr(1));
}

std::vector<TokenInfo> Assembler::parseTokens(const std::vector<std::string>& tokens) {
    std::vector<TokenInfo> parsed;

    auto parseToken = [&](const std::string& token) {
        TokenInfo info;

        if (token.empty()) throw std::runtime_error("Invalid Token: <empty>");

        if (token[0] == 'W' || token[0] == 'w') {
            info.isReg = true;
            info.isX = false;
        } else if (token[0] == 'X' || token[0] == 'x') {
            info.isReg = true;
            info.isX = true;
        } else if (token.rfind("#0x", 0) == 0) {
            info.isImm = true;
            info.isHex = true;
        } else if (token.rfind("#", 0) == 0) {
            info.isImm = true;
            info.isHex = false;
        } else if (token != "SP" && token != "sp") {
            throw std::runtime_error("Invalid Token: " + token);
        }

        info.token = token;
        info.isValid = true;
        parsed.push_back(info);
    };

    for (const auto& token : tokens) {
        parseToken(token);
    }

    return parsed;
}
