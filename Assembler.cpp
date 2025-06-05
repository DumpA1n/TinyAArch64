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
    {"EOR", {OP_EOR, OP_EORI}}
};

static std::unordered_map<std::string, ConditionCode> B_COND_Map = {
    {"B.EQ", COND_EQ},
    {"B.NE", COND_NE},
    {"B.LT", COND_LT},
    {"B.GE", COND_GE},
    {"B.GT", COND_GT},
    {"B.LE", COND_LE},
    {"B.AL", COND_AL},
    {"BEQ",  COND_EQ},
    {"BNE",  COND_NE},
    {"BLT",  COND_LT},
    {"BGE",  COND_GE},
    {"BGT",  COND_GT},
    {"BLE",  COND_LE},
    {"BAL",  COND_AL}
};

std::vector<uint32_t> Assembler::assemble(const std::vector<std::string>& asmLines) {
    std::vector<uint32_t> machineCode;
    std::unordered_map<std::string, int> labelAddresses;
    std::vector<std::pair<int, std::string>> pendingLabels;

    int pc = 0;

    // 第一次遍历：处理标签
    for (const auto& line : asmLines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

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
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            tokens.push_back(token);
        }
        std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), 
                        [](unsigned char c) { return std::toupper(c); });

        if (tokens.empty()) throw std::runtime_error("None operands");

        std::string opcode = tokens[0];

        if (opcode == "MOV") {
            if (tokens.size() < 3) throw std::runtime_error("Too few operands, opcode: " + opcode);
            uint32_t instr;
            if (tokens[2].front() == 'R') {
                uint8_t rd_num = parseReg(tokens[1]);
                uint8_t rn_num = parseReg(tokens[2]);
                instr = (OP_MOV << 26) | (rd_num << 21) | (rn_num << 16);
            } else if (tokens[2].front() == '#') {
                uint8_t rd_num = parseReg(tokens[1]);
                bool isHex = false;
                if (tokens[2].rfind("#0x", 0) == 0 || tokens[2].rfind("#0X", 0) == 0)
                    isHex = true;
                uint32_t imm_val = std::stoul(tokens[2].substr(1), nullptr, isHex ? 16 : 10);
                instr = (OP_MOVI << 26) | (rd_num << 21) | (0 << 16) | imm_val;
            } else {
                std::cerr << "Invalid Instruction: " << trimmed << std::endl;
            }
            machineCode.push_back(instr);

        } else if (opcode == "ADD" || opcode == "SUB" || opcode == "AND" || opcode == "ORR" || opcode == "EOR") {
            if (tokens.size() < 4) throw std::runtime_error("Too few operands, opcode: " + opcode);
            uint32_t instr;
            if (tokens[3].front() == 'R') {
                uint8_t rd_num = parseReg(tokens[1]);
                uint8_t rn_num = parseReg(tokens[2]);
                uint8_t rm_num = parseReg(tokens[3]);
                instr = (OpcodeMap[opcode].first << 26) | (rd_num << 21) | (rn_num << 16) | rm_num;
            } else if (tokens[3].front() == '#') {
                uint8_t rd_num = parseReg(tokens[1]);
                uint8_t rn_num = parseReg(tokens[2]);
                bool isHex = false;
                if (tokens[3].rfind("#0x", 0) == 0 || tokens[3].rfind("#0X", 0) == 0)
                    isHex = true;
                uint32_t imm_val = std::stoul(tokens[3].substr(1), nullptr, isHex ? 16 : 10);
                instr = (OpcodeMap[opcode].second << 26) | (rd_num << 21) | (rn_num << 16) | imm_val;
            } else {
                std::cerr << "Invalid Instruction: " << trimmed << std::endl;
            }
            machineCode.push_back(instr);

        } else if (opcode == "MUL" || opcode == "SDIV" || opcode == "UDIV") {
            if (tokens.size() < 4) throw std::runtime_error("Too few operands, opcode: " + opcode);
            uint8_t rd_num = parseReg(tokens[1]);
            uint8_t rn_num = parseReg(tokens[2]);
            uint8_t rm_num = parseReg(tokens[3]);
            uint32_t instr = (OpcodeMap[opcode].first << 26) | (rd_num << 21) | (rn_num << 16) | rm_num;
            machineCode.push_back(instr);

        } else if (opcode == "CMP") {
            if (tokens.size() < 3) throw std::runtime_error("Too few operands, opcode: " + opcode);
            uint8_t rd_num = parseReg(tokens[1]);
            uint8_t rn_num = parseReg(tokens[2]);
            uint32_t instr = (OP_CMP << 26) | (rd_num << 21) | (rn_num << 16);
            machineCode.push_back(instr);

        // } else if (opcode == "STR") {
        //     std::string rt, rest;
        //     iss >> rt >> rest;
        //     size_t comma = rest.find(',');
        //     std::string baseReg = rest.substr(1, comma - 1);
        //     std::string offsetStr = rest.substr(comma + 2, rest.length() - comma - 3);
        //     uint8_t rt_num = parseReg(rt);
        //     uint8_t rn_num = parseReg(baseReg);
        //     uint16_t offset = stoi(offsetStr);
        //     uint32_t instr = (OP_STR << 26) | (rt_num << 21) | (rn_num << 16) | offset;
        //     machineCode.push_back(instr);

        } else if (opcode == "B") {
            if (tokens.size() < 2) throw std::runtime_error("Too few operands, opcode: " + opcode);
            std::string label = tokens[1];
            pendingLabels.push_back({pc, label});
            uint32_t instr = (OP_B << 26);
            machineCode.push_back(instr); // 占位，后续填充

        } else if (B_COND_Map.find(opcode) != B_COND_Map.end()) {
            if (tokens.size() < 2) throw std::runtime_error("Too few operands, opcode: " + opcode);
            std::string label = tokens[1];
            pendingLabels.push_back({pc, label});
            uint8_t condition = B_COND_Map[opcode];
            uint32_t instr = (OP_B_COND << 26) | (((condition & 0x0F) << 22));
            machineCode.push_back(instr); // 占位，后续填充

        } else if (opcode == "HLT") {
            uint32_t instr = (OP_HLT << 26);
            machineCode.push_back(instr);

        } else {
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
            // machineCode[addr / 4] = (OP_B << 26) | (offset & 0x03FFFFFF);
            machineCode[addr / 4] |= (offset & 0x03FFFFFF);
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
    if (r[0] != 'R' && r[0] != 'r') throw std::runtime_error("无效寄存器名: " + r);
    return stoi(r.substr(1));
}
