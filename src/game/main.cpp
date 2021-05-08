#include <iostream>
#include <raylib.h>

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include <imgui.h>
#include <cimgui_impl_raylib.hpp>

using namespace std;

int main()
{
	InitWindow( 1280, 720, "Game" );
	SetWindowState( FLAG_WINDOW_RESIZABLE );

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplRaylib_Init();
	Texture2D fontTexture;
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels = NULL;
		int width, height;
		int bytesPerPixel;
		io.Fonts->AddFontDefault();
		io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height, &bytesPerPixel );

		Image image;
		memset( &image, 0, sizeof( Image ) );
		image.width = width;
		image.height = height;
		image.mipmaps = 1;
		image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
		image.data = RL_MALLOC( width * height * bytesPerPixel );
		memcpy( image.data, pixels, width * height * bytesPerPixel );

		fontTexture = LoadTextureFromImage( image );
		io.Fonts->SetTexID( &fontTexture.id );
		UnloadImage( image );
	}

	while ( !WindowShouldClose() )
	{
		ImGui_ImplRaylib_NewFrame();
		ImGui_ImplRaylib_ProcessEvent();
		ImGui::NewFrame();

		BeginDrawing();
		{
			ClearBackground( RAYWHITE );

			ImGui::ShowDemoWindow();
			ImGui::Render();
			raylib_render_cimgui( ImGui::GetDrawData() );
		}
		EndDrawing();
	}

	return 0;
}
