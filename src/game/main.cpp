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
	const int tileSize;
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

	Camera2D camera;
	memset( &camera, 0, sizeof( Camera2D ) );
	camera.zoom = 64;

	while ( !WindowShouldClose() )
	{
		ImGui_ImplRaylib_NewFrame();
		ImGui_ImplRaylib_ProcessEvent();
		ImGui::NewFrame();

		if ( ImGui::Begin( "Game" ) )
		{
			if ( ImGui::TreeNode( "Camera" ) )
			{
				ImGui::DragFloat2( "Offset", &camera.offset.x );
				ImGui::DragFloat2( "Target", &camera.target.x, 0.1f );
				ImGui::DragFloat( "Rotation", &camera.rotation );
				ImGui::DragFloat( "Zoom", &camera.zoom );
				if ( ImGui::Button( "Reset" ) )
				{
					memset( &camera, 0, sizeof( Camera2D ) );
					camera.zoom = 64;
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();

		BeginDrawing();
		{
			ClearBackground( RAYWHITE );

			BeginMode2D( camera );

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
