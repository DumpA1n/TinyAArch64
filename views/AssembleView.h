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

        static bool codeReadOnly = false;
        static std::vector<std::string> lines;
        static std::vector<int> displayLineIndex;
        static std::unordered_map<uint64_t, int> pcToLineMap;

        ImGui::Begin("AssembleView");

        CPU& cpu = CPU::GetInstance();
        Assembler Asm;

        if (ImGui::Button("Start")) {
            cpu.reset();
            auto binary = Asm.assemble(asmCode);
            cpu.loadProgram(binary);
            LOGI(LOG_INSTANCE("CPU"), "===== Starting CPU Simulation =====");
            cpu.printState();

            lines.clear();
            displayLineIndex.clear();
            pcToLineMap.clear();

            std::istringstream iss(asmCode);
            std::string line;
            while (std::getline(iss, line)) {
                std::string trimmed = Assembler::trim(line);
                if (trimmed.empty() || trimmed.rfind("//", 0) == 0) continue;
                lines.push_back(trimmed);
            }

            // 建立 pc -> 行号 映射，忽略 label 行
            uint64_t pc = 0;
            for (size_t i = 0; i < lines.size(); ++i) {
                int lastLineIndex = displayLineIndex.empty() ? 0 : displayLineIndex.back();
                int displayLine = (i > 0 && lines[i - 1].back() == ':') ? lastLineIndex : lastLineIndex + 1;

                displayLineIndex.push_back(displayLine);

                if (lines[i].back() == ':') {
                    continue;
                }
                pcToLineMap[pc++] = displayLineIndex.size();
            }

            codeReadOnly = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            cpu.reset();
            codeReadOnly = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Execute")) {
            try {
                for (int i = 0; i < 99999; i++) {
                    LOGI(LOG_INSTANCE("CPU"), ">>> Step %d <<<", ++cpu.steps);
                    cpu.step();
                }
            } catch (const std::exception& e) {
                LOGI(LOG_INSTANCE("CPU"), "Execution stopped: %s", e.what());
            }
            LOGI(LOG_INSTANCE("CPU"), "===== Simulation Finished =====");
        }

        ImGui::SameLine();
        if (ImGui::Button("Next")) {
            try {
                LOGI(LOG_INSTANCE("CPU"), ">>> Step %d <<<", ++cpu.steps);
                cpu.step();
            } catch (const std::exception& e) {
                LOGI(LOG_INSTANCE("CPU"), "Execution stopped: %s", e.what());
            }
        }

        if (!codeReadOnly) {
            ImGui::InputTextMultiline("##asm", asmCode, IM_ARRAYSIZE(asmCode),
                                    ImVec2(-1.0f, -1.0f));
        } else {
            ImGui::BeginChild("AsmReadOnly", ImVec2(-1, -1), true, ImGuiWindowFlags_HorizontalScrollbar);

            int pc = cpu.getPC() / 4;
            int currentExecLine = pcToLineMap.count(pc) ? pcToLineMap[pc] - 1: -1;

            for (int i = 0; i < lines.size(); ++i) {
                bool isCurrentExec = (i == currentExecLine);
                ImVec4 color = isCurrentExec ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
                                            : ImGui::GetStyleColorVec4(ImGuiCol_Text);
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                if (lines[i].back() == ':')
                    ImGui::Text("%s", lines[i].c_str());
                else
                    ImGui::Text("%4d | %s", displayLineIndex[i], lines[i].c_str());
                ImGui::PopStyleColor();
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
};
