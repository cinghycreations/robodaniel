#pragma once

struct ImDrawData;

bool ImGui_ImplRaylib_Init();
void ImGui_ImplRaylib_Shutdown();
void ImGui_ImplRaylib_NewFrame();
bool ImGui_ImplRaylib_ProcessEvent();
void raylib_render_cimgui( ImDrawData* draw_data );
