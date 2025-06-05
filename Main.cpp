#include "CPU.h"

static int test_address;

// ========================== 测试程序 ==========================
std::vector<uint32_t> createTestProgram() {
    // 测试程序：计算斐波那契数列前10项
    std::vector<uint32_t> program;
    Assembler Asm;

    std::vector<std::string> asmcode {
        "MOV R0, #0x1",
        "MOV R1, R0",
        // "MOV R1, #1",
        // "ADD R0, R0, R1",
        // "MOV R1, #" + std::to_string(&test_address),
        "B Lable",
        "MOV R26, #1111",
        "MOV R27, #111",
        "MOV R28, #11",
        "Lable:",
        "HLT"
    };

    return Asm.assemble(asmcode);
    
    // // 手动汇编的机器码
    // program.push_back(0b00100100000000010000000000000000); // MOV R1, #0      (R1 = 0)
    // program.push_back(0b00100100000000100000000000000001); // MOV R2, #1      (R2 = 1)
    // program.push_back(0b00100100000000110000000000000000); // MOV R3, #0      (R3 = 0, 计数器)
    // program.push_back(0b00100100000001000000000000001010); // MOV R4, #10     (R4 = 10, 循环次数)
    
    // // 存储斐波那契数列的起始地址 (0x100)
    // program.push_back(0b00100100000001010000000000000001); // MOV R5, #1      (R5 = 1)
    // program.push_back(0b00100100000001100000000100000000); // MOV R6, #0x100  (R6 = 0x100)
    
    // // 存储前两个值
    // program.push_back(0b00101100000001100101000000000000); // STR R5, [R6, #0] (存储1)
    // program.push_back(0b00100100000001010000000000000010); // MOV R5, #2      (R5 = 2, 地址偏移)
    // program.push_back(0b00101100000001100101000000000100); // STR R2, [R6, #4] (存储1)
    
    // // 循环开始
    // // loop:
    // program.push_back(0b00000000001000100000000000000000); // ADD R0, R1, R2  (R0 = R1 + R2)
    // program.push_back(0b00101100000001100100000000000000); // STR R0, [R6, R5, LSL #2] (存储结果)
    // program.push_back(0b00000000001000000001000000000000); // MOV R1, R2      (R1 = R2)
    // program.push_back(0b00000000010000000010000000000000); // MOV R2, R0      (R2 = R0)
    // program.push_back(0b00000000001001010010100000000000); // ADD R5, R5, #1  (R5 += 1)
    // program.push_back(0b00000000011001000001100000000000); // ADD R3, R3, #1  (R3 += 1)
    // program.push_back(0b00111000000001000011000000000000); // CMP R3, R4      (比较计数器和循环次数)
    // program.push_back(0b00110101000000000000000000011000); // B.LT loop       (如果R3 < R4则跳转)
    
    // // 结束
    // program.push_back(0b11111100000000000000000000000000); // HLT
    
    return program;
}

// ========================== 主函数 ==========================
int main() {
    CPU cpu;
    
    try {
        // 创建并加载测试程序
        std::vector<uint32_t> testProgram = createTestProgram();
        cpu.loadProgram(testProgram);
        // return 0;
        std::cout << "===== Starting CPU Simulation =====" << std::endl;
        std::cout << "Loaded program with " << testProgram.size() << " instructions" << std::endl << std::endl;
        
        // 初始状态
        cpu.printState();
        
        // 执行程序
        try {
            for (int i = 0; i < 50; i++) { // 最多执行50条指令
                std::cout << ">>> Step " << (i + 1) << " <<<" << std::endl;
                cpu.step();
                cpu.printState();
            }
        } 
        catch (const std::exception& e) {
            std::cout << "Execution stopped: " << e.what() << std::endl;
        }
        
        std::cout << "===== Simulation Finished =====" << std::endl;
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}