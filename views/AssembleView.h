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

        CPU& cpu = CPU::GetInstance();
        Assembler Asm;

        if (ImGui::Button("Start")) {
            cpu.reset();
            cpu.loadProgram(Asm.assemble(asmCode));
            LOGI(LOG_INSTANCE("CPU"), "===== Starting CPU Simulation =====");
            cpu.printState();
        }
        ImGui::SameLine();
        if (ImGui::Button("Execute")) {
            try {
                for (int i = 0; i < 99999; i++) {
                    LOGI(LOG_INSTANCE("CPU"), ">>> Step %d <<<", ++cpu.steps);
                    cpu.step();
                }
            }
            catch (const std::exception& e) {
                LOGI(LOG_INSTANCE("CPU"), "Execution stopped: %s", e.what());
            }
            LOGI(LOG_INSTANCE("CPU"), "===== Simulation Finished =====");
        }
        ImGui::SameLine();
        if (ImGui::Button("Next")) {
            try {
                LOGI(LOG_INSTANCE("CPU"), ">>> Step %d <<<", ++cpu.steps);
                cpu.step();
            }
            catch (const std::exception& e) {
                LOGI(LOG_INSTANCE("CPU"), "Execution stopped: %s", e.what());
            }
        }

        ImGui::InputTextMultiline("##asm", asmCode, IM_ARRAYSIZE(asmCode),
                                ImVec2(-1.0f, -1.0f));

        ImGui::End();
    }
};
