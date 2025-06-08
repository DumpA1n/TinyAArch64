#pragma once

#include "View.h"
#include "CPU.h"
#include "imgui.h"
#include <iomanip>
#include <sstream>
#include <vector>

class MemoryView : public View {
public:
    MemoryView() {}

    void Show() {
        CPU& cpu = CPU::GetInstance();
        const auto memory = cpu.getMemory();
        constexpr size_t bytesPerRow = 32;
        constexpr size_t memSizeToShow = std::min(cpu.MEM_SIZE, 512ULL);
        static uint8_t prevMemory[memSizeToShow] = {0};
        static int highlightTimer[memSizeToShow] = {0};
        constexpr int kHighlightFrames = 60;

        ImGui::SetNextWindowSize(ImVec2{getViewSize().w, getViewSize().h});
        ImGui::SetNextWindowPos(ImVec2{getViewPos().x, getViewPos().y});

        ImGui::Begin("MemoryView");
        ImGui::BeginChild("MemoryRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        static size_t memBase = 0;
        static char addrInput[32] = "0x0";
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("##AddressInput", addrInput, IM_ARRAYSIZE(addrInput));
        ImGui::SameLine();
        if (ImGui::Button("JumpToAddress")) {
            std::string str(addrInput);
            if (str.compare("SP") == 0 || str.compare("sp") == 0) {
                memBase = cpu.getSP();
            } else {
                memBase = std::stoull(addrInput, nullptr, 0);
            }
        }

        if (ImGui::BeginTable("MemoryTable", bytesPerRow + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Address");
            for (int i = 0; i < bytesPerRow; ++i)
                ImGui::TableSetupColumn(std::to_string(i).c_str());

            ImGui::TableHeadersRow();

            for (size_t row = 0; row < memSizeToShow / bytesPerRow; ++row) {
                size_t baseAddr = row * bytesPerRow + memBase;
                ImGui::TableNextRow();

                for (int col = 0; col <= bytesPerRow; ++col) {
                    ImGui::TableSetColumnIndex(col);
                    if (col == 0) {
                        ImGui::Text("0x%04X", (unsigned int)baseAddr);
                    } else {
                        size_t index = baseAddr + (col - 1);
                        if (index >= memSizeToShow || index >= memory.size()) continue;

                        uint8_t value = memory[index];

                        // // 高亮内存变化
                        // if (value != prevMemory[index]) {
                        //     highlightTimer[index] = kHighlightFrames;
                        //     prevMemory[index] = value;
                        // }

                        // if (highlightTimer[index] > 0) {
                        //     ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
                        //     highlightTimer[index]--;
                        // }

                        ImGui::Text("%02X", value);

                        // if (highlightTimer[index] > 0) {
                        //     ImGui::PopStyleColor();
                        // }
                    }
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
        ImGui::End();
    }
};
