#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <raylib.h>
#include <intmath.hpp>
#include <queue>
#include <filesystem>
#include <functional>

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include <imgui.h>
#include <cimgui_impl_raylib.hpp>

#pragma optimize("",off)

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

	static int getEmpty()
	{
		return -1;
	}

	static bool isGround( const int tile )
	{
		return tile >= 0 && tile < 32;
	}

	static bool isImpassable( const int tile )
	{
		return isGround( tile );
	}

	static int getHero()
	{
		return 32;
	}

	static int getCoin()
	{
		return 96;
	}

	static int getClosedExit()
	{
		return 97;
	}

	static int getOpenExit()
	{
		return 98;
	}

	static int getEnemy()
	{
		return 128;
	}

	static bool isEnemyBlueprint( const int tile )
	{
		return tile >= 256 && tile <= 272;
	}

	static void getEnemyBlueprintProperties( const int tile, int& pathLength, bool& horizontal, bool& startsAtEnd )
	{
		switch ( tile )
		{
		case 257:
			pathLength = 3;
			horizontal = false;
			startsAtEnd = false;
			break;

		case 258:
			pathLength = 4;
			horizontal = false;
			startsAtEnd = false;
			break;

		case 259:
			pathLength = 6;
			horizontal = false;
			startsAtEnd = false;
			break;

		case 260:
			pathLength = 12;
			horizontal = false;
			startsAtEnd = false;
			break;

		case 261:
			pathLength = 3;
			horizontal = true;
			startsAtEnd = false;
			break;

		case 262:
			pathLength = 4;
			horizontal = true;
			startsAtEnd = false;
			break;

		case 263:
			pathLength = 6;
			horizontal = true;
			startsAtEnd = false;
			break;

		case 264:
			pathLength = 12;
			horizontal = true;
			startsAtEnd = false;
			break;

		case 265:
			pathLength = 3;
			horizontal = true;
			startsAtEnd = true;
			break;

		case 266:
			pathLength = 4;
			horizontal = true;
			startsAtEnd = true;
			break;

		case 267:
			pathLength = 6;
			horizontal = true;
			startsAtEnd = true;
			break;

		case 268:
			pathLength = 12;
			horizontal = true;
			startsAtEnd = true;
			break;

		case 269:
			pathLength = 3;
			horizontal = false;
			startsAtEnd = true;
			break;

		case 270:
			pathLength = 4;
			horizontal = false;
			startsAtEnd = true;
			break;

		case 271:
			pathLength = 6;
			horizontal = false;
			startsAtEnd = true;
			break;

		case 272:
			pathLength = 12;
			horizontal = false;
			startsAtEnd = true;
			break;
		}
	}

private:
	Texture texture;
	Vector2Int tilesPerSide;
};

class Level
{
public:
	Level( const filesystem::path& path )
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
		if ( coords.x < 0 || coords.x >= size.x || coords.y < 0 || coords.y >= size.y )
		{
			throw std::exception( "Coords out of bounds" );
		}

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

		return Vector2Int{ -1, -1 };
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

struct PathPoint
{
	Vector2 coords;
	float progress;
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
	Pathfinder( const Tiles& _tiles, const Level& _level ) : tiles( _tiles ), level( _level ) { }

	vector<PathPoint> goTo( const Vector2Int& currentPosition, const Vector2Int& destination )
	{
		if ( Vector2IntEqual( currentPosition, destination ) )
		{
			return vector<PathPoint>();
		}

		struct CellState
		{
			float shortestPath = -1;
			vector<Vector2Int> trajectory;
		};

		vector<CellState> cellStates( level.getSize().x * level.getSize().y );
		auto cellAt = [ &cellStates, mapWidth = level.getSize().x ]( const Vector2Int& position )->CellState& { return cellStates.at( position.y * mapWidth + position.x ); };

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
					if ( position.y + 1 >= level.getSize().y || !Tiles::isImpassable( level.getCellAt( Vector2Int{ position.x, position.y + 1 } ) ) )
					{
						continue;
					}
				}

				vector<Vector2Int> trajectory;
				trajectory.push_back( position );

				for ( const Vector2Int& delta : move.steps )
				{
					const Vector2Int stepPosition = Vector2IntAdd( position, delta );
					if ( stepPosition.x < 0 || stepPosition.x >= level.getSize().x || stepPosition.y < 0 || stepPosition.y >= level.getSize().y )
					{
						break;
					}
					if ( Tiles::isImpassable( level.getCellAt( stepPosition ) ) )
					{
						break;
					}

					trajectory.push_back( stepPosition );
				}

				float currentPathLength = currentCell.shortestPath;
				for ( int step = 1; step < trajectory.size(); ++step )
				{
					currentPathLength += Vector2Distance( Vector2IntToFloat( trajectory.at( step - 1 ) ), Vector2IntToFloat( trajectory.at( step ) ) );

					CellState& cell = cellAt( trajectory.at( step ) );
					if ( cell.shortestPath < 0 || currentPathLength < cell.shortestPath )
					{
						cell.shortestPath = currentPathLength;
						cell.trajectory = trajectory;

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
			return vector<PathPoint>();
		}

		vector<Vector2Int> pathPositions;
		{
			Vector2Int position = destination;
			while ( true )
			{
				pathPositions.push_back( position );
				if ( Vector2IntEqual( position, currentPosition ) )
				{
					break;
				}

				position = cellAt( position ).trajectory.front();
			}

			std::reverse( pathPositions.begin(), pathPositions.end() );
		}

		vector<PathPoint> trajectory;
		trajectory.push_back( PathPoint{ Vector2IntToFloat( currentPosition ), 0 } );
		float progressOffset = 0;
		for ( int i = 1; i < pathPositions.size(); ++i )
		{
			const CellState& cell = cellAt( pathPositions.at( i ) );
			vector<PathPoint> smoothPath = catmullClark( cell.trajectory, 2, progressOffset );
			trajectory.insert( trajectory.end(), smoothPath.begin() + 1, smoothPath.end() );
			progressOffset += cell.trajectory.size() - 1;
		}

		return trajectory;
	}

private:
	const Tiles& tiles;
	const Level& level;

	vector<PathPoint> catmullClark( const vector<Vector2Int> path, const int iterations, const float progressOffset )
	{
		vector<PathPoint> points;
		for ( int i = 0; i < path.size(); ++i )
		{
			points.push_back( PathPoint{ Vector2IntToFloat( path.at( i ) ), progressOffset + i } );
		}

		if ( points.size() < 3 )
		{
			return points;
		}

		int iterationsToDo = iterations;
		while ( iterationsToDo-- )
		{
			vector<PathPoint> midpoints;
			midpoints.reserve( points.size() - 1 );
			for ( int i = 0; i < points.size() - 1; ++i )
			{
				PathPoint midpoint;
				midpoint.coords = Vector2Scale( Vector2Add( points.at( i ).coords, points.at( i + 1 ).coords ), 0.5f );
				midpoint.progress = ( points.at( i ).progress + points.at( i + 1 ).progress ) / 2;
				midpoints.push_back( midpoint );
			}

			vector<PathPoint> subdivided;
			subdivided.reserve( points.size() + midpoints.size() );
			subdivided.push_back( points.front() );
			for ( int i = 0; i < midpoints.size() - 1; ++i )
			{
				subdivided.push_back( midpoints.at( i ) );

				PathPoint newPoint;
				newPoint.coords.x = points.at( i + 1 ).coords.x * 0.5f + midpoints.at( i ).coords.x * 0.25f + midpoints.at( i + 1 ).coords.x * 0.25f;
				newPoint.coords.y = points.at( i + 1 ).coords.y * 0.5f + midpoints.at( i ).coords.y * 0.25f + midpoints.at( i + 1 ).coords.y * 0.25f;
				newPoint.progress = points.at( i + 1 ).progress;
				subdivided.push_back( newPoint );
			}
			subdivided.push_back( midpoints.back() );
			subdivided.push_back( points.back() );

			points = std::move( subdivided );
		}

		return points;
	}
};

struct Settings
{
	struct
	{
		float heroStepsPerSecond = 16;
		float coinRadius = 0.4f;
		float enemySpeed = 4;
		float enemyRadius = 0.5f;
	} gameplay;

	struct
	{
		bool enableDebugCamera = false;
		Camera2D debugCamera{ Vector2Zero(), Vector2Zero(), 0, 64 };
		bool pathDebugDraw = false;
	} debug;
};

class Session
{
public:

	struct Enemy
	{
		Vector2Int startCell;
		Vector2Int endCell;
		float progress = 0;
		Vector2 position;
	};

	const Tiles& tiles;
	Level& level;
	const Settings& settings;

	Camera2D gameplayCamera;

	Vector2Int heroTile;
	Vector2 heroPosition;

	Pathfinder pathfinder;
	vector<PathPoint> currentPath;
	float progress = 0;

	int totalCoins = 0;
	int collectedCoins = 0;

	bool completed = false;
	bool failed = false;

	vector<Enemy> enemies;

	float totalTime = 0;

	Session( const Tiles& _tiles, Level& _level, const Settings& _settings ) : tiles( _tiles ), level( _level ), settings( _settings ), pathfinder( tiles, level )
	{
		memset( &gameplayCamera, 0, sizeof( Camera2D ) );

		heroTile = level.findFirstCell( Tiles::getHero() );
		level.setCellAt( heroTile, Tiles::getEmpty() );
		heroPosition = Vector2IntToFloat( heroTile );

		totalCoins = level.findAllCells( Tiles::getCoin() ).size();

		createEnemies();
	}

	void step()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if ( ImGui::BeginMenu( "Session" ) )
			{
				if ( ImGui::MenuItem( "Complete" ) )
				{
					completed = true;
					return;
				}
				if ( ImGui::MenuItem( "Fail" ) )
				{
					failed = true;
					return;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		totalTime += GetFrameTime();

		// Set camera
		gameplayCamera.target = Vector2{ float( level.getSize().x ) / 2, float( level.getSize().y ) / 2 };
		gameplayCamera.offset = Vector2{ float( GetScreenWidth() ) / 2, float( GetScreenHeight() ) / 2 };
		gameplayCamera.zoom = std::min<float>( float( GetScreenWidth() ) / level.getSize().x, float( GetScreenHeight() ) / level.getSize().y );

		// Find new path
		if ( IsMouseButtonPressed( MOUSE_LEFT_BUTTON ) && !ImGui::GetIO().WantCaptureMouse )
		{
			const Vector2 worldPosition = GetScreenToWorld2D( GetMousePosition(), settings.debug.enableDebugCamera ? settings.debug.debugCamera : gameplayCamera );
			const Vector2Int currentPosition{ int( heroPosition.x ), int( heroPosition.y ) };
			const Vector2Int destination{ int( worldPosition.x ), int( worldPosition.y ) };
			currentPath = pathfinder.goTo( currentPosition, destination );
			progress = 0;
		}

		// Move hero
		if ( !currentPath.empty() )
		{
			progress += settings.gameplay.heroStepsPerSecond * GetFrameTime();
			if ( progress >= currentPath.back().progress )
			{
				heroPosition = currentPath.back().coords;
				currentPath.clear();
				progress = 0;
			} else
			{
				for ( int i = 0; i < currentPath.size() - 1; ++i )
				{
					if ( progress >= currentPath.at( i ).progress && progress < currentPath.at( i + 1 ).progress )
					{
						const float blend = ( progress - currentPath.at( i ).progress ) / ( currentPath.at( i + 1 ).progress - currentPath.at( i ).progress );
						heroPosition = Vector2Lerp( currentPath.at( i ).coords, currentPath.at( i + 1 ).coords, blend );
					}
				}
			}
		}

		// Move enemies
		updateEnemies( GetFrameTime() );

		// Check collisions
		{
			const Vector2 heroCenter{ heroPosition.x + 0.5f, heroPosition.y + 0.5f };
			const Vector2Int touchedTile{ int( heroCenter.x ), int( heroCenter.y ) };
			const Vector2 touchedTileCenter{ float( touchedTile.x ) + 0.5f, float( touchedTile.y ) + 0.5f };
			const float distance = Vector2Distance( heroCenter, touchedTileCenter );

			if ( level.getCellAt( touchedTile ) == Tiles::getCoin() && distance <= settings.gameplay.coinRadius )
			{
				level.setCellAt( touchedTile, Tiles::getEmpty() );
				++collectedCoins;
			}

			if ( level.getCellAt( touchedTile ) == Tiles::getOpenExit() && distance <= 0.1f )
			{
				completed = true;
			}

			for ( const Enemy& enemy : enemies )
			{
				if ( Vector2Distance( heroCenter, enemy.position ) <= settings.gameplay.enemyRadius )
				{
					failed = true;
				}
			}
		}

		// Open exit
		if ( collectedCoins == totalCoins )
		{
			const Vector2Int doorPosition = level.findFirstCell( Tiles::getClosedExit() );
			if ( doorPosition.x != -1 && doorPosition.y != -1 )
			{
				level.setCellAt( doorPosition, Tiles::getOpenExit() );
			}
		}
	}

	void render()
	{
		// Draw world
		for ( int i = 0; i < level.getSize().y; ++i )
		{
			for ( int j = 0; j < level.getSize().x; ++j )
			{
				const int cell = level.getCellAt( Vector2Int{ j, i } );
				if ( cell != Tiles::getEmpty() )
				{
					DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( cell ), Rectangle{ float( j ), float( i ), 1, 1 }, Vector2{ 0, 0 }, 0, WHITE );
				}
			}
		}

		// Draw hero
		DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( Tiles::getHero() ), Rectangle{ heroPosition.x, heroPosition.y, 1, 1 }, Vector2{ 0, 0 }, 0, WHITE );

		// Draw enemies
		for ( const Enemy& enemy : enemies )
		{
			DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( Tiles::getEnemy() ), Rectangle{ enemy.position.x - 0.5f, enemy.position.y - 0.5f, 1, 1 }, Vector2{ 0,0 }, 0, WHITE );
		}
	}

private:

	void createEnemies()
	{
		for ( int i = 0; i < level.getSize().y; ++i )
		{
			for ( int j = 0; j < level.getSize().x; ++j )
			{
				const Vector2Int cellPosition{ j, i };
				const int cell = level.getCellAt( cellPosition );
				if ( Tiles::isEnemyBlueprint( cell ) )
				{
					int pathLength;
					bool horizontal;
					bool startsAtEnd;
					Tiles::getEnemyBlueprintProperties( cell, pathLength, horizontal, startsAtEnd );

					Enemy enemy;
					enemy.startCell = cellPosition;
					if ( horizontal )
					{
						enemy.endCell = Vector2Int{ cellPosition.x + pathLength - 1, cellPosition.y };
					} else
					{
						enemy.endCell = Vector2Int{ cellPosition.x, cellPosition.y - ( pathLength - 1 ) };
					}
					if ( startsAtEnd )
					{
						std::swap( enemy.startCell, enemy.endCell );
					}
					enemy.progress = 0;

					enemies.push_back( enemy );
					level.setCellAt( cellPosition, Tiles::getEmpty() );
				}
			}
		}

		updateEnemies( 0 );
	}

	void updateEnemies( const float deltaTime )
	{
		for ( Enemy& enemy : enemies )
		{
			const Vector2 startPosition = Vector2Add( Vector2IntToFloat( enemy.startCell ), Vector2{ 0.5f, 0.5f } );
			const Vector2 endPosition = Vector2Add( Vector2IntToFloat( enemy.endCell ), Vector2{ 0.5f, 0.5f } );
			const float halfLength = Vector2Distance( startPosition, endPosition );
			const float fullLength = halfLength * 2;

			enemy.progress += settings.gameplay.enemySpeed * deltaTime;
			enemy.progress = std::fmod( enemy.progress, fullLength );

			if ( enemy.progress < halfLength )
			{
				enemy.position = Vector2Lerp( startPosition, endPosition, enemy.progress / halfLength );
			} else
			{
				enemy.position = Vector2Lerp( endPosition, startPosition, ( enemy.progress - halfLength ) / halfLength );
			}
		}
	}
};

class GameFlow
{
public:
	const Tiles& tiles;
	const Settings& settings;

	GameFlow( const Tiles& _tiles, const Settings& _settings ) : tiles( _tiles ), settings( _settings )
	{
		currentHandler = &GameFlow::splashScreen;
	}

	void step()
	{
		currentHandler( this );
	}

private:
	function<void( GameFlow* )> currentHandler;

	unique_ptr<Level> level;
	unique_ptr<Session> session;
	int nextLevel = 0;

	void splashScreen()
	{
		if ( ImGui::Begin( "Splash Screen", false, ImGuiWindowFlags_NoDecoration ) )
		{
			ImGui::Text( "Press start to begin" );
			if ( ImGui::Button( "Start" ) )
			{
				nextLevel = 0;
				currentHandler = &GameFlow::initSession;
			}
		}
		ImGui::End();
	}

	void initSession()
	{
		filesystem::path levelPath = string( "level" ) + to_string( nextLevel ) + ".csv";
		level.reset( new Level( levelPath ) );
		session.reset( new Session( tiles, *level, settings ) );

		currentHandler = &GameFlow::play;
	}

	void play()
	{
		// Step session
		session->step();

		// Render UI
		{
			ImGui::SetNextWindowPos( ImVec2( 30, 30 ) );
			if ( ImGui::Begin( "HUD", false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize ) )
			{
				ImGui::Text( "Coins: %d / %d", session->collectedCoins, session->totalCoins );
				ImGui::Text( "Time %.3f", session->totalTime );
			}
			ImGui::End();
		}

		// Render level
		BeginMode2D( settings.debug.enableDebugCamera ? settings.debug.debugCamera : session->gameplayCamera );
		session->render();

		if ( settings.debug.pathDebugDraw && !session->currentPath.empty() )
		{
			for ( int i = 0; i < session->currentPath.size() - 1; ++i )
			{
				DrawLineV( Vector2{ float( session->currentPath.at( i ).coords.x ) + 0.5f, float( session->currentPath.at( i ).coords.y ) + 0.5f },
					Vector2{ float( session->currentPath.at( i + 1 ).coords.x ) + 0.5f, float( session->currentPath.at( i + 1 ).coords.y ) + 0.5f }, BLUE );
			}
		}

		EndMode2D();

		if ( session->completed )
		{
			currentHandler = &GameFlow::sessionCompleted;
		} else if ( session->failed )
		{
			currentHandler = &GameFlow::sessionFailed;
		}
	}

	void sessionCompleted()
	{
		BeginMode2D( settings.debug.enableDebugCamera ? settings.debug.debugCamera : session->gameplayCamera );
		session->render();
		EndMode2D();

		if ( ImGui::Begin( "Session completed", false, ImGuiWindowFlags_NoDecoration ) )
		{
			ImGui::Text( "Level completed in %.3f seconds! Press continue", session->totalTime );
			if ( ImGui::Button( "Continue" ) )
			{
				session.reset();
				level.reset();

				++nextLevel;
				filesystem::path levelPath = string( "level" ) + to_string( nextLevel ) + ".csv";
				if ( !filesystem::exists( levelPath ) )
				{
					nextLevel = 0;
					currentHandler = &GameFlow::splashScreen;
				} else
				{
					currentHandler = &GameFlow::initSession;
				}
			}
		}
		ImGui::End();
	}

	void sessionFailed()
	{
		BeginMode2D( settings.debug.enableDebugCamera ? settings.debug.debugCamera : session->gameplayCamera );
		session->render();
		EndMode2D();

		if ( ImGui::Begin( "Session failed", false, ImGuiWindowFlags_NoDecoration ) )
		{
			ImGui::Text( "Level failed! Press retry" );
			if ( ImGui::Button( "Retry" ) )
			{
				session.reset();
				level.reset();

				currentHandler = &GameFlow::initSession;
			}
		}
		ImGui::End();
	}
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
		extern unsigned int droidsans_compressed_size;
		extern unsigned int droidsans_compressed_data[];

		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels = NULL;
		int width, height;
		int bytesPerPixel;
		io.Fonts->AddFontFromMemoryCompressedTTF( droidsans_compressed_data, droidsans_compressed_size, 18.0f );
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

	Tiles tiles( "tiles.png", 64 );
	Settings settings;
	GameFlow flow( tiles, settings );
	bool showSettings = false;

	while ( !WindowShouldClose() )
	{
		ImGui_ImplRaylib_NewFrame();
		ImGui_ImplRaylib_ProcessEvent();
		ImGui::NewFrame();

		if ( ImGui::BeginMainMenuBar() )
		{
			if ( ImGui::BeginMenu( "Game" ) )
			{
				ImGui::MenuItem( "Show Settings", nullptr, &showSettings );
				if ( ImGui::MenuItem( "Quit" ) )
				{
					break;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if ( showSettings )
		{
			if ( ImGui::Begin( "Settings" ) )
			{
				if ( ImGui::CollapsingHeader( "Gameplay" ) )
				{
					ImGui::DragFloat( "Steps per Second", &settings.gameplay.heroStepsPerSecond, 0.01f );
					ImGui::SliderFloat( "Coin Collision Radius", &settings.gameplay.coinRadius, 0, 0.5f );
					ImGui::DragFloat( "Enemy Speed", &settings.gameplay.enemySpeed, 0.01f );
					ImGui::SliderFloat( "Enemy Collision Radius", &settings.gameplay.enemyRadius, 0, 0.5f );
				}

				if ( ImGui::CollapsingHeader( "Debug" ) )
				{
					ImGui::Checkbox( "Enable Debug Camera", &settings.debug.enableDebugCamera );
					if ( settings.debug.enableDebugCamera )
					{
						ImGui::DragFloat2( "Offset", &settings.debug.debugCamera.offset.x );
						ImGui::DragFloat2( "Target", &settings.debug.debugCamera.target.x, 0.1f );
						ImGui::DragFloat( "Rotation", &settings.debug.debugCamera.rotation );
						ImGui::DragFloat( "Zoom", &settings.debug.debugCamera.zoom );
						if ( ImGui::Button( "Reset" ) )
						{
							memset( &settings.debug.debugCamera, 0, sizeof( Camera2D ) );
							settings.debug.debugCamera.zoom = 64;
						}
					}
					ImGui::Checkbox( "Hero Path", &settings.debug.pathDebugDraw );
				}
			}
			ImGui::End();
		}

		BeginDrawing();
		{
			ClearBackground( RAYWHITE );
			flow.step();

			ImGui::Render();
			raylib_render_cimgui( ImGui::GetDrawData() );
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
