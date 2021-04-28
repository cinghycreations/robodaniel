#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <raylib.h>
#include <intmath.hpp>
#include <queue>

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include <imgui.h>
#include <cimgui_impl_raylib.hpp>

#pragma optimize("",off)

using namespace std;

enum
{
	Tile_Empty = -1,
	Tile_Ground,
	Tile_Sky,
	Tile_Hero,
	Tile_Coin,
	Tile_Enemy,
};

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

	void setCellAt( const Vector2Int& coords, const int tile )
	{
		const int cellIndex = coords.y * size.x + coords.x;
		cells.at( cellIndex ) = tile;
	}

	Vector2Int findFirstCell( const int tile )
	{
		for ( int i = 0; i < size.y; ++i )
		{
			for ( int j = 0; j < size.x; ++j )
			{
				const Vector2Int coords{ j, i };
				if ( getCellAt( coords ) == tile )
				{
					return coords;
				}
			}
		}
	}

	vector<Vector2Int> findAllCells( const int tile )
	{
		vector<Vector2Int> result;

		for ( int i = 0; i < size.y; ++i )
		{
			for ( int j = 0; j < size.x; ++j )
			{
				const Vector2Int coords{ j, i };
				if ( getCellAt( coords ) == tile )
				{
					result.push_back( coords );
				}
			}
		}

		return result;
	}

private:
	Vector2Int size;
	vector<int> cells;
};

class Pathfinder
{
private:

	enum MoveFlags
	{
		MoveFlags_None = 0x0,
		MoveFlags_NeedsSolidBottom = 0x1,
	};

	struct Move
	{
		string description;
		unsigned int flags;
		vector<Vector2Int> steps;
	};

	const vector<Move> moves
	{
		{ "Right", MoveFlags_NeedsSolidBottom, { { 1, 0 } } },
		{ "Right Long Jump", MoveFlags_NeedsSolidBottom, { { 1, -1 }, { 2, -1 }, { 3, 0 } } },
		{ "Right High Jump", MoveFlags_NeedsSolidBottom, { { 0, -1 }, { 0, -2 }, { 0, -3 }, { 1, -3 } } },
		{ "Left", MoveFlags_NeedsSolidBottom, { { -1, 0 } } },
		{ "Left Long Jump", MoveFlags_NeedsSolidBottom, { { -1, -1 }, { -2, -1 }, { -3, 0 } } },
		{ "Left High Jump", MoveFlags_NeedsSolidBottom, { { 0, -1 }, { 0, -2 }, { 0, -3 }, { -1, -3 } } },
		{ "Gravity", MoveFlags_None, { { 0, 1 } } },
	};

public:
	Pathfinder( const Tiles& _tiles, const Map& _map ) : tiles( _tiles ), map( _map ) { }

	vector<Vector2Int> goTo( const Vector2Int& currentPosition, const Vector2Int& destination )
	{
		if ( Vector2IntEqual( currentPosition, destination ) )
		{
			return vector<Vector2Int>();
		}

		struct CellState
		{
			float shortestPath = -1;
			Vector2Int previousPosition;
			vector<Vector2Int> trajectoryFromPreviousPosition;
		};

		vector<CellState> cellStates( map.getSize().x * map.getSize().y );
		auto cellAt = [ &cellStates, mapWidth = map.getSize().x ]( const Vector2Int& position )->CellState& { return cellStates.at( position.y * mapWidth + position.x ); };

		queue<Vector2Int> positionsToVisit;
		cellAt( currentPosition ).shortestPath = 0;
		positionsToVisit.push( currentPosition );

		while ( !positionsToVisit.empty() )
		{
			Vector2Int position = positionsToVisit.front();
			positionsToVisit.pop();

			CellState& currentCell = cellAt( position );
			for ( const Move& move : moves )
			{
				if ( move.flags & MoveFlags_NeedsSolidBottom )
				{
					if ( position.y + 1 >= map.getSize().y || map.getCellAt( Vector2Int{ position.x, position.y + 1 } ) != Tile_Ground )
					{
						continue;
					}
				}

				vector<Vector2Int> trajectory;

				for ( const Vector2Int& delta : move.steps )
				{
					const Vector2Int stepPosition = Vector2IntAdd( position, delta );
					if ( stepPosition.x < 0 || stepPosition.x >= map.getSize().x || stepPosition.y < 0 || stepPosition.y >= map.getSize().y )
					{
						break;
					}
					if ( map.getCellAt( stepPosition ) == Tile_Ground )
					{
						break;
					}

					trajectory.push_back( stepPosition );
				}

				float currentPathLength = currentCell.shortestPath;
				Vector2Int lastPosition = position;
				for ( int step = 0; step < trajectory.size(); ++step )
				{
					currentPathLength += Vector2Distance( Vector2IntToFloat( lastPosition ), Vector2IntToFloat( trajectory.at( step ) ) );
					lastPosition = trajectory.at( step );

					CellState& cell = cellAt( trajectory.at( step ) );
					if ( cell.shortestPath < 0 || currentPathLength < cell.shortestPath )
					{
						cell.shortestPath = currentPathLength;
						cell.previousPosition = position;
						cell.trajectoryFromPreviousPosition = trajectory;

						if ( step == trajectory.size() - 1 )
						{
							positionsToVisit.push( trajectory.at( step ) );
						}
					}
				}
			}
		}

		const CellState& destinationCell = cellAt( destination );
		if ( destinationCell.shortestPath == -1 )
		{
			return vector<Vector2Int>();
		}

		vector<Vector2Int> reverseTrajectory;
		{
			Vector2Int position = destination;
			while ( !Vector2IntEqual( position, currentPosition ) )
			{
				const CellState& cell = cellAt( position );
				reverseTrajectory.insert( reverseTrajectory.end(), cell.trajectoryFromPreviousPosition.rbegin(), cell.trajectoryFromPreviousPosition.rend() );
				position = cell.previousPosition;
			}
		}

		vector<Vector2Int> trajectoryToDestination( reverseTrajectory.rbegin(), reverseTrajectory.rend() );
		return trajectoryToDestination;
	}

private:
	const Tiles& tiles;
	const Map& map;
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
	Map testbed( "testbed.csv" );

	Camera2D gameplayCamera;
	memset( &gameplayCamera, 0, sizeof( Camera2D ) );

	Camera2D debugCamera;
	memset( &debugCamera, 0, sizeof( Camera2D ) );
	debugCamera.zoom = 64.0f;

	bool enableDebugCamera = false;

	Vector2Int heroTile = testbed.findFirstCell( Tile_Hero );
	testbed.setCellAt( heroTile, Tile_Empty );
	Vector2 heroPosition{ heroTile.x, heroTile.y };

	Pathfinder pathfinder( tiles, testbed );

	float secondsPerStep = 0.2f;
	vector<Vector2Int> currentPath;
	float progress = 0;

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

		if ( IsMouseButtonPressed( MOUSE_LEFT_BUTTON ) && !ImGui::GetIO().WantCaptureMouse )
		{
			const Vector2 worldPosition = GetScreenToWorld2D( GetMousePosition(), enableDebugCamera ? debugCamera : gameplayCamera );
			const Vector2Int currentPosition{ int( heroPosition.x ), int( heroPosition.y ) };
			const Vector2Int destination{ int( worldPosition.x ), int( worldPosition.y ) };
			currentPath = pathfinder.goTo( currentPosition, destination );
			progress = 0;
		}

		BeginDrawing();
		{
			ClearBackground( RAYWHITE );

			BeginMode2D( enableDebugCamera ? debugCamera : gameplayCamera );

			for ( int i = 0; i < testbed.getSize().y; ++i )
			{
				for ( int j = 0; j < testbed.getSize().x; ++j )
				{
					const int cell = testbed.getCellAt( Vector2Int{ j, i } );
					if ( cell != Tile_Empty )
					{
						DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( cell ), Rectangle{ float( j ), float( i ), 1, 1 }, Vector2{ 0, 0 }, 0, WHITE );
					}
				}
			}
			DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( Tile_Hero ), Rectangle{ heroPosition.x, heroPosition.y, 1, 1 }, Vector2{ 0,0 }, 0, WHITE );

			if ( !currentPath.empty() )
			{
				DrawLineV( Vector2{ float( heroPosition.x ) + 0.5f, float( heroPosition.y ) + 0.5f }, Vector2{ float( currentPath.front().x ) + 0.5f, float( currentPath.front().y ) + 0.5f }, BLUE );
				for ( int i = 0; i < currentPath.size() - 1; ++i )
				{
					DrawLineV( Vector2{ float( currentPath.at( i ).x ) + 0.5f, float( currentPath.at( i ).y ) + 0.5f }, Vector2{ float( currentPath.at( i + 1 ).x ) + 0.5f, float( currentPath.at( i + 1 ).y ) + 0.5f }, BLUE );
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
