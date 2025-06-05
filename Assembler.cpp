#include "Assembler.h"

#include "CPU.h"

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
        if (trimmed.empty() || trimmed.back() == ':') continue;
        std::cout << "trimmed: " << trimmed << std::endl;

        std::istringstream iss(trimmed);
        std::string opcode;
        iss >> opcode;

        if (opcode == "MOV") {
            // std::string rd, imm;
            // iss >> rd >> imm;
            // uint8_t rd_num = parseReg(rd);
            // uint16_t imm_val = stoi(imm.substr(1));
            // uint32_t instr = (0b001001 << 26) | (rd_num << 21) | (0 << 16) | imm_val;
            // machineCode.push_back(instr);
            uint32_t instr;
            std::vector<std::string> tokens(2);
            iss >> tokens[0] >> tokens[1];
            std::cout << tokens[0] << " " << tokens[1] << std::endl;
            if (tokens[1].front() == 'R') {
                uint8_t rd_num = parseReg(tokens[0]);
                uint8_t rn_num = parseReg(tokens[1]);
                instr = (OP_MOV << 26) | (rd_num << 21) | (rn_num << 16);
            } else if (tokens[1].front() == '#') {
                uint8_t rd_num = parseReg(tokens[0]);
                bool isHex = false;
                if (tokens[1].rfind("#0x", 0) == 0 || tokens[1].rfind("#0X", 0) == 0)
                    isHex = true;
                uint32_t imm_val = std::stoul(tokens[1].substr(1), nullptr, isHex ? 16 : 10);
                instr = (OP_MOVI << 26) | (rd_num << 21) | (0 << 16) | imm_val;
            } else {
                std::cerr << "Invalid Instruction: " << trimmed << std::endl;
            }
            machineCode.push_back(instr);
        } else if (opcode == "ADD") {
            std::string rd, rn, rm;
            iss >> rd >> rn >> rm;
            uint8_t rd_num = parseReg(rd);
            uint8_t rn_num = parseReg(rn);
            uint8_t rm_num = parseReg(rm);
            uint32_t instr = (OP_ADD << 26) | (rd_num << 21) | (rn_num << 16) | rm_num;
            machineCode.push_back(instr);
        } else if (opcode == "SUB") {
            std::string rd, rn, rm;
            iss >> rd >> rn >> rm;
            uint8_t rd_num = parseReg(rd);
            uint8_t rn_num = parseReg(rn);
            uint8_t rm_num = parseReg(rm);
            uint32_t instr = (OP_SUB << 26) | (rd_num << 21) | (rn_num << 16) | rm_num;
            machineCode.push_back(instr);
        } else if (opcode == "STR") {
            std::string rt, rest;
            iss >> rt >> rest;
            size_t comma = rest.find(',');
            std::string baseReg = rest.substr(1, comma - 1);
            std::string offsetStr = rest.substr(comma + 2, rest.length() - comma - 3);
            uint8_t rt_num = parseReg(rt);
            uint8_t rn_num = parseReg(baseReg);
            uint16_t offset = stoi(offsetStr);
            uint32_t instr = (OP_STR << 26) | (rt_num << 21) | (rn_num << 16) | offset;
            machineCode.push_back(instr);
        } else if (opcode == "B") {
            std::string label;
            iss >> label;
            pendingLabels.push_back({pc, label});
            machineCode.push_back(0); // 占位，后续填充
        } else if (opcode == "HLT") {
            uint32_t instr = (OP_HLT << 26);
            machineCode.push_back(instr);
        } else {
            std::cerr << "未知指令: " << opcode << std::endl;
            exit(1);
        }

        uint32_t code = machineCode[machineCode.size() - 1];
        for (int i = 31; i >= 0; --i) {
            std::cout << ((code >> i) & 1);
        }
        std::cout << std::endl;

        pc += 4;
    }

    // 填补跳转地址
    for (auto& [addr, label] : pendingLabels) {
        if (labelAddresses.count(label)) {
            int labelAddr = labelAddresses[label];
            int offset = (labelAddr - (addr + 4)) >> 2;
            machineCode[addr / 4] = (OP_B << 26) | (offset & 0x03FFFFFF);
        } else {
            std::cerr << "未知标签: " << label << std::endl;
            exit(1);
        }
    }

    return machineCode;
}

std::string Assembler::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

uint8_t Assembler::parseReg(const std::string& r) {
    if (r[0] != 'R') throw std::runtime_error("无效寄存器名: " + r);
    return stoi(r.substr(1));
}
