#include "CPU.h"

static int test_address;

// ========================== 测试程序 ==========================
std::vector<uint32_t> createTestProgram() {
    // 测试程序：计算斐波那契数列前10项
    std::vector<uint32_t> program;
    Assembler Asm;

    std::string shellcode = R"(
        mov w0, #-2
        mov w1, #4
        add w0, w0, #12
        cmp w0, w1
        b.le L_cmp
        mul w2, w0, w1
        b L_exit
    L_cmp:
        mov w3, #10
    L_exit:
        hlt
    )";

    return Asm.assemble(shellcode);
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