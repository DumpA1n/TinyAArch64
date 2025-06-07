#pragma once

#include "View.h"
#include "Log.h"
#include "CPU.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class AssembleView : public View {
public:
    AssembleView() {}
    void Show() {
        ImGui::SetNextWindowSize(ImVec2{getViewSize().w, getViewSize().h});
        ImGui::SetNextWindowPos(ImVec2{getViewPos().x, getViewPos().y});

        static char asmCode[10240] = 
            "MOV W0, #1\n"
            "ADD W1, W0, #2\n"
            "B LABEL\n"
            "LABEL:\n"
            "HLT\n";

        ImGui::Begin("AssembleView");

        ImGui::InputTextMultiline("##asm", asmCode, IM_ARRAYSIZE(asmCode),
                                ImVec2(-1.0f, -1.0f));

        if (ImGui::Button("Execute")) {
            CPU& cpu = CPU::GetInstance();
            cpu.reset();
            Assembler Asm;
            cpu.loadProgram(Asm.assemble(asmCode));

            LOGI(LOG_INSTANCE("CPU"), "===== Starting CPU Simulation =====");
            cpu.printState();
            try {
                for (int i = 0; i < 99999; i++) {
                    LOGI(LOG_INSTANCE("CPU"), ">>> Step %d <<<", (i + 1));
                    cpu.step();
                    cpu.printRegisterState();
                    cpu.printRegisterState2();
                }
            }
            catch (const std::exception& e) {
                if (std::string(e.what()).compare("HLT instruction executed") == 0) {
                    cpu.printRegisterState();
                    cpu.printRegisterState2();
                }
                std::cout << "Execution stopped: " << e.what() << std::endl;
            }
            LOGI(LOG_INSTANCE("CPU"), "===== Simulation Finished =====");
        }

        ImGui::End();
    }
};
