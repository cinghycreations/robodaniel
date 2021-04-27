#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <raylib.h>
#include <intmath.hpp>

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include <imgui.h>
#include <cimgui_impl_raylib.hpp>

using namespace std;

class Tiles
{
public:
	const int tileSize;

	Tiles( const string& path, const int _tileSize ) : tileSize( _tileSize )
	{
		texture = LoadTexture( path.c_str() );
		tilesPerSide.x = texture.width / tileSize;
		tilesPerSide.y = texture.height / tileSize;
	}

	~Tiles()
	{
		UnloadTexture( texture );
	}

	const Texture& getTexture() const
	{
		return texture;
	}

	Rectangle getRectangleForTile( const int tile ) const
	{
		Rectangle rectangle;
		rectangle.x = ( tile % tilesPerSide.x ) * tileSize;
		rectangle.y = ( tile / tilesPerSide.x ) * tileSize;
		rectangle.width = tileSize;
		rectangle.height = tileSize;
		return rectangle;
	}

private:
	Texture texture;
	Vector2Int tilesPerSide;
};

class Map
{
public:
	Map( const string& path )
	{
		ifstream stream( path );
		int rows = 0;
		string line;
		while ( std::getline( stream, line ) )
		{
			++rows;

			stringstream lineStream( line );
			string svalue;

			while ( std::getline( lineStream, svalue, ',' ) )
			{
				const int value = std::stoi( svalue );
				cells.push_back( value );
			}
		}

		size.y = rows;
		size.x = cells.size() / rows;
	}

	const Vector2Int getSize() const
	{
		return size;
	}

	int getCellAt( const Vector2Int& coords ) const
	{
		const int cellIndex = coords.y * size.x + coords.x;
		return cells.at( cellIndex );
	}

private:
	Vector2Int size;
	vector<int> cells;
};

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
		io.Fonts->AddFontDefault();
		io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height, nullptr );

		Image image;
		memset( &image, 0, sizeof( Image ) );
		image.width = width;
		image.height = height;
		image.mipmaps = 1;
		image.format = UNCOMPRESSED_R8G8B8A8;
		image.data = RL_MALLOC( width * height * 4 );
		memcpy( image.data, pixels, width * height * 4 );

		fontTexture = LoadTextureFromImage( image );
		io.Fonts->SetTexID( &fontTexture.id );
		UnloadImage( image );
	}

	Tiles tiles( "tiles.png", 64 );
	Map testbed( "level0.csv" );

	Camera2D gameplayCamera;
	memset( &gameplayCamera, 0, sizeof( Camera2D ) );

	Camera2D debugCamera;
	memset( &debugCamera, 0, sizeof( Camera2D ) );
	debugCamera.zoom = 64.0f;

	bool enableDebugCamera = false;

	while ( !WindowShouldClose() )
	{
		ImGui_ImplRaylib_NewFrame();
		ImGui_ImplRaylib_ProcessEvent();
		ImGui::NewFrame();

		if ( ImGui::Begin( "Game" ) )
		{
			if ( ImGui::CollapsingHeader( "Camera" ) )
			{
				ImGui::Checkbox( "Enable Debug Camera", &enableDebugCamera );
				if ( enableDebugCamera )
				{
					ImGui::DragFloat2( "Offset", &debugCamera.offset.x );
					ImGui::DragFloat2( "Target", &debugCamera.target.x, 0.1f );
					ImGui::DragFloat( "Rotation", &debugCamera.rotation );
					ImGui::DragFloat( "Zoom", &debugCamera.zoom );
					if ( ImGui::Button( "Reset" ) )
					{
						memset( &debugCamera, 0, sizeof( Camera2D ) );
						debugCamera.zoom = 64;
					}
				}
			}
		}
		ImGui::End();

		gameplayCamera.target = Vector2{ float( testbed.getSize().x ) / 2, float( testbed.getSize().y ) / 2 };
		gameplayCamera.offset = Vector2{ float( GetScreenWidth() ) / 2, float( GetScreenHeight() ) / 2 };
		gameplayCamera.zoom = std::min<float>( float( GetScreenWidth() ) / testbed.getSize().x, float( GetScreenHeight() ) / testbed.getSize().y );

		BeginDrawing();
		{
			ClearBackground( RAYWHITE );

			BeginMode2D( enableDebugCamera ? debugCamera : gameplayCamera );

			for ( int i = 0; i < testbed.getSize().y; ++i )
			{
				for ( int j = 0; j < testbed.getSize().x; ++j )
				{
					const int cell = testbed.getCellAt( Vector2Int{ j, i } );
					if ( cell != -1 )
					{
						DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( cell ), Rectangle{ float( j ), float( i ), 1, 1 }, Vector2{ 0, 0 }, 0, WHITE );
					}
				}
			}

			EndMode2D();

			ImGui::Render();
			raylib_render_cimgui( ImGui::GetDrawData() );
		}
		EndDrawing();
	}

	return 0;
}
