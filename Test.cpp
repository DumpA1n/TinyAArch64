#include "CPU.h"

// ========================== 测试程序 ==========================
std::vector<uint32_t> createTestProgram() {
    std::vector<uint32_t> program;
    Assembler Asm;

    // std::string shellcode = R"(
    // start:
    //     ldr w5, [x0, #0x4]
    //     add w6, w5, #1
    //     str w6, [x0, #0x60]
    //     ldr w7, [x0, #0x60]
    //     mov w0, #-2
    //     mov w1, #4
    //     add w0, w0, #12
    //     cmp w0, w1
    //     b.le L_cmp
    //     mul w2, w0, w1
    //     b L_exit
    // L_cmp:
    //     mov w3, #10
    // L_exit:
    //     hlt
    // data_i:
    //     .int 123 
    // data_f:
    //     .float 1.23
    // )";

    // std::string shellcode = R"(
    //     sub     sp, sp, #16
    //     mov     w0, #0
    //     str     w0, [sp, #12]
    //     str     w0, [sp, #8]
    //     b       .L2
    // .L3:
    //     ldr     w1, [sp, #12]
    //     ldr     w0, [sp, #8]
    //     add     w0, w1, w0
    //     str     w0, [sp, #12]
    //     ldr     w0, [sp, #8]
    //     add     w0, w0, #1
    //     str     w0, [sp, #8]
    // .L2:
    //     ldr     w0, [sp, #8]
    //     cmp     w0, #100
    //     ble     .L3
    //     add     sp, sp, #16
    //     HLT
    // )";

    
    // 计算斐波那契数列第N项
    // 50 12586269025 0x2EE333961
    // const int N = 51;
    // uint64_t fib[N];
    // fib[0] = 0;
    // fib[1] = 1;
    // for (int i = 2; i < N; ++i) {
    //     fib[i] = fib[i - 1] + fib[i - 2];
    //     printf("fib[%d] = %llX %llu\n", i, fib[i], fib[i]);
    // }

    std::string shellcode = R"(
        sub     sp, sp, #16
        mov     x0, #0
        mov     x1, #1
        mov     x3, #1
    .Lloop:
        add     x2, x0, x1
        // bl      Add()
        // mov     x2, x0
        mov     x0, x1
        mov     x1, x2
        add     x3, x3, #1
        cmp     x3, #50
        ble     .Lloop
        add     sp, sp, #16
        HLT
    Add():
        add     x0, x0, x1
        ret
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
            for (int i = 0; i < 99999; i++) { // 最多执行50条指令
                std::cout << ">>> Step " << (i + 1) << " <<<" << std::endl;
                cpu.step();
                // cpu.printState();
            }
        } 
        catch (const std::exception& e) {
            if (std::string(e.what()).compare("HLT instruction executed") == 0) {
                cpu.printState();
            }
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