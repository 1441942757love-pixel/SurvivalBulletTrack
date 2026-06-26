#ifndef ImGuiAndroid_Drawing
#define ImGuiAndroid_Drawing

#include "../ImGui/imgui_internal.h"

namespace Drawing { 

  void DrawLine(ImVec2 start, ImVec2 end, float thickness, ImColor color) {
     auto background = ImGui::GetBackgroundDrawList();
     if(background) {
       background->AddLine(start, end, color, thickness);
     }
   }
    
   void DrawBox(Rect rect, float thickness, ImColor color) {
        ImVec2 v1(rect.x, rect.y);
        ImVec2 v2(rect.x + rect.width, rect.y);
        ImVec2 v3(rect.x + rect.width, rect.y + rect.height);
        ImVec2 v4(rect.x, rect.y + rect.height);

        DrawLine(v1, v2, thickness, color);
        DrawLine(v2, v3, thickness, color);
        DrawLine(v3, v4, thickness, color);
        DrawLine(v4, v1, thickness, color);
    }

    void DrawCornerBox(ImColor color, float thickness, Rect rect) {
        DrawLine(ImVec2(rect.x, rect.y), ImVec2(rect.x + (rect.width / 3), rect.y), thickness, color);
        DrawLine(ImVec2(rect.x + rect.width, rect.y), ImVec2((rect.x + rect.width) - (rect.width / 3), rect.y), thickness, color);
        DrawLine(ImVec2(rect.x, rect.y + rect.height), ImVec2((rect.x + (rect.width / 3)), rect.y + rect.height), thickness, color);
        DrawLine(ImVec2(rect.x + rect.width, rect.y + rect.height), ImVec2((rect.x + rect.width) - (rect.width / 3), rect.y + rect.height), thickness, color);
        DrawLine(ImVec2(rect.x, rect.y), ImVec2(rect.x, rect.y + (rect.height / 6)), thickness, color);
        DrawLine(ImVec2(rect.x + rect.width, rect.y), ImVec2(rect.x + rect.width, rect.y + (rect.height / 6)), thickness, color);
        DrawLine(ImVec2(rect.x, rect.y + rect.height), ImVec2(rect.x, (rect.y + rect.height) - (rect.height / 6)), thickness, color);
        DrawLine(ImVec2(rect.x + rect.width, rect.y + rect.height), ImVec2(rect.x + rect.width, (rect.y + rect.height) - (rect.height / 6)), thickness, color);             
    }
    
    void DrawText(float fontSize, ImVec2 position, ImColor color, const char *text, bool centered = false, bool outline = false, ImColor outlineColor = ImColor(0, 0, 0, 255)) {
    auto background = ImGui::GetBackgroundDrawList();
    if(!background || !text) return;

    // Calculate text size if centering is needed
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 textPos = position;
    
    if (centered) {
        textPos.x -= textSize.x * 0.5f;
        textPos.y -= textSize.y * 0.5f;
    }

    // Draw outline if enabled
    if (outline) {
        const float outlineThickness = 1.0f;
        for (float x = -outlineThickness; x <= outlineThickness; x += outlineThickness) {
            for (float y = -outlineThickness; y <= outlineThickness; y += outlineThickness) {
                if (x != 0 || y != 0) { // Skip the center position
                    background->AddText(ImGui::GetFont(), fontSize, 
                                       ImVec2(textPos.x + x, textPos.y + y), 
                                       outlineColor, text);
                }
            }
        }
    }

    // Draw main text
    background->AddText(ImGui::GetFont(), fontSize, textPos, color, text);
}
    
    void DrawCircle(ImVec2 end, float radius, bool filled, int thickness, ImColor color) {
        auto background = ImGui::GetBackgroundDrawList();

        if(background) {
            if(filled) {
                background->AddCircleFilled(end, radius, color);
            } else {
                background->AddCircle(end, radius, color, 100, thickness);
            }
        }
    }
}

#endif