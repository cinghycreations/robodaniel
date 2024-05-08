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
#include <optional>
#include <array>

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include <imgui.h>
#include <rlImGui.h>
#include <nlohmann/json.hpp>

using namespace std;

class BaseException : public std::exception
{
public:
	BaseException( const string& _message ) : message( _message ) { }

	const char* what() const noexcept override { return message.c_str(); }

protected:
	const string message;
};

namespace ImGui {
	void CenterWindowForText( const string& text )
	{
		const ImVec2 textSize = ImGui::CalcTextSize( text.c_str() );
		const float windowWidth = textSize.x + 100;
		const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowPos( ImVec2( displaySize.x * 0.5f - windowWidth / 2, displaySize.y * 0.1f ) );
		ImGui::SetNextWindowSize( ImVec2( windowWidth, 0 ) );
	}

	bool CenteredButton( const string& label )
	{
		const ImVec2 textSize = ImGui::CalcTextSize( label.c_str() );
		ImGui::NewLine();
		ImGui::SameLine( ( ImGui::GetContentRegionAvail().x - textSize.x ) / 2 );
		return ImGui::Button( label.c_str() );
	}

	void CenteredText( const string& label )
	{
		const ImVec2 textSize = ImGui::CalcTextSize( label.c_str() );
		ImGui::NewLine();
		ImGui::SameLine( ( ImGui::GetContentRegionAvail().x - textSize.x ) / 2 );
		ImGui::Text( label.c_str() );
	}

	void CenteredTextDisabled( const string& label )
	{
		const ImVec2 textSize = ImGui::CalcTextSize( label.c_str() );
		ImGui::NewLine();
		ImGui::SameLine( ( ImGui::GetContentRegionAvail().x - textSize.x ) / 2 );
		ImGui::TextDisabled( label.c_str() );
	}

#ifdef __RELEASE
	bool BeginDevMenuBar()
	{
		return false;
	}
#else
	bool BeginDevMenuBar()
	{
		return BeginMainMenuBar();
	}
#endif
}

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
		return tile == 0 || tile == 1;
	}

	static bool isImpassable( const int tile )
	{
		return isGround( tile );
	}

	static int getHero()
	{
		return 32;
	}

	static const vector<int>& getHeroIdleAnimation()
	{
		static const vector<int> animation{ 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 };
		return animation;
	}

	static const vector<int>& getHeroIdleMirroredAnimation()
	{
		static const vector<int> animation{ 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
		return animation;
	}

	static const vector<int>& getHeroJumpAnimation()
	{
		static const vector<int> animation{ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };
		return animation;
	}

	static const vector<int>& getHeroJumpMirroredAnimation()
	{
		static const vector<int> animation{ 62, 63, 64, 65, 66, 67, 68, 69, 70, 71 };
		return animation;
	}

	static const vector<int>& getHeroRunAnimation()
	{
		static const vector<int> animation{ 72, 73, 74, 75, 76, 77, 78, 79 };
		return animation;
	}

	static const vector<int>& getHeroRunMirroredAnimation()
	{
		static const vector<int> animation{ 80, 81, 82, 83, 84, 85, 86, 87 };
		return animation;
	}

	static int getCoin()
	{
		return 2;
	}

	static int getClosedExit()
	{
		return 3;
	}

	static int getOpenExit()
	{
		return 4;
	}

	static int getEnemy()
	{
		return 5;
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
			throw BaseException( "Coords out of bounds" );
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
	unsigned int moveFlags;
};

enum MoveFlags
{
	MoveFlags_None = 0x0,
	MoveFlags_NeedsSolidBottom = 0x1,
	MoveFlags_JumpAnimation = 0x2,
	MoveFlags_MirroredAnimation = 0x4,
};

class Pathfinder
{
private:
	struct Move
	{
		string description;
		unsigned int flags;
		vector<Vector2Int> steps;
	};

	const vector<Move> moves
	{
		{ "Right", MoveFlags_NeedsSolidBottom, { { 1, 0 } } },
		{ "Right Long Jump", MoveFlags_NeedsSolidBottom | MoveFlags_JumpAnimation, { { 1, -1 }, { 2, -1 }, { 3, 0 } } },
		{ "Right High Jump", MoveFlags_NeedsSolidBottom | MoveFlags_JumpAnimation, { { 0, -1 }, { 0, -2 }, { 0, -3 }, { 1, -3 } } },
		{ "Left", MoveFlags_NeedsSolidBottom | MoveFlags_MirroredAnimation, { { -1, 0 } } },
		{ "Left Long Jump", MoveFlags_NeedsSolidBottom | MoveFlags_MirroredAnimation | MoveFlags_JumpAnimation, { { -1, -1 }, { -2, -1 }, { -3, 0 } } },
		{ "Left High Jump", MoveFlags_NeedsSolidBottom | MoveFlags_MirroredAnimation | MoveFlags_JumpAnimation, { { 0, -1 }, { 0, -2 }, { 0, -3 }, { -1, -3 } } },
		{ "Gravity", MoveFlags_JumpAnimation, { { 0, 1 } } },
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
			unsigned int moveFlags;
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
						cell.moveFlags = move.flags;

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
		Vector2Int lastPosition = currentPosition;
		for ( int i = 1; i < pathPositions.size(); ++i )
		{
			const CellState& cell = cellAt( pathPositions.at( i ) );
			vector<PathPoint> smoothPath = catmullClark( cell.trajectory, 2, progressOffset, cell.moveFlags );
			trajectory.insert( trajectory.end(), smoothPath.begin() + 1, smoothPath.end() );
			progressOffset += cell.trajectory.size() - 1;
			lastPosition = cell.trajectory.back();
		}

		while ( true )
		{
			const Vector2Int below{ lastPosition.x, lastPosition.y + 1 };
			if ( below.y >= level.getSize().y )
			{
				trajectory.push_back( PathPoint{ Vector2IntToFloat( below ), trajectory.back().progress + 1, MoveFlags_JumpAnimation } );
				break;
			} else if ( !Tiles::isImpassable( level.getCellAt( below ) ) )
			{
				trajectory.push_back( PathPoint{ Vector2IntToFloat( below ), trajectory.back().progress + 1, MoveFlags_JumpAnimation } );
				lastPosition = below;
				continue;
			} else
			{
				break;
			}
		}

		return trajectory;
	}

private:
	const Tiles& tiles;
	const Level& level;

	vector<PathPoint> catmullClark( const vector<Vector2Int> path, const int iterations, const float progressOffset, const unsigned int moveFlags )
	{
		vector<PathPoint> points;
		for ( int i = 0; i < path.size(); ++i )
		{
			points.push_back( PathPoint{ Vector2IntToFloat( path.at( i ) ), progressOffset + i, moveFlags } );
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
				midpoint.moveFlags = moveFlags;
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
				newPoint.moveFlags = moveFlags;
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
		float heroAnimationFps = 10.0f;
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

enum class Language
{
	English,
	Italian,
};

class Translator
{
public:
	void setLanguage( const Language _language )
	{
		language = _language;

		string languageCode;
		if ( language == Language::Italian )
		{
			languageCode = "it";
		}

		nlohmann::json allLanguages;
		ifstream stream( "languages.json" );
		stream >> allLanguages;

		database = nlohmann::json{};
		if ( allLanguages.count( languageCode ) )
		{
			database = allLanguages.at( languageCode );
		}
	}

	Language getLanguage() const
	{
		return language;
	}

	string translate( const string& source ) const
	{
		return database.count( source ) ? database.at( source ).get<string>() : source;
	}

private:

	Language language = Language::English;
	nlohmann::json database;
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
	unsigned int heroMoveFlags;

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
		if ( ImGui::BeginDevMenuBar() )
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

			if ( !( destination.x < 0 || destination.x >= level.getSize().x || destination.y < 0 || destination.y >= level.getSize().y ) )
			{
				currentPath = pathfinder.goTo( currentPosition, destination );
				progress = 0;
				heroMoveFlags = MoveFlags_None;
			}
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
				heroMoveFlags = MoveFlags_None;
			} else
			{
				for ( int i = 0; i < currentPath.size() - 1; ++i )
				{
					if ( progress >= currentPath.at( i ).progress && progress < currentPath.at( i + 1 ).progress )
					{
						const float blend = ( progress - currentPath.at( i ).progress ) / ( currentPath.at( i + 1 ).progress - currentPath.at( i ).progress );
						heroPosition = Vector2Lerp( currentPath.at( i ).coords, currentPath.at( i + 1 ).coords, blend );
						heroMoveFlags = currentPath.at( i + 1 ).moveFlags;
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

			if ( touchedTile.x < 0 || touchedTile.x >= level.getSize().x || touchedTile.y < 0 || touchedTile.y >= level.getSize().y )
			{
				failed = true;
			} else
			{
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
		// Draw enemies
		for ( const Enemy& enemy : enemies )
		{
			DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( Tiles::getEnemy() ), Rectangle{ enemy.position.x - 0.5f, enemy.position.y - 0.5f, 1, 1 }, Vector2{ 0,0 }, 0, WHITE );
		}

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
		{
			const vector<int>* animation;

			if ( currentPath.empty() )
			{
				animation = &Tiles::getHeroIdleAnimation();
			} else if ( heroMoveFlags & MoveFlags_JumpAnimation )
			{
				if ( heroMoveFlags & MoveFlags_MirroredAnimation )
				{
					animation = &Tiles::getHeroJumpMirroredAnimation();
				} else
				{
					animation = &Tiles::getHeroJumpAnimation();
				}
			} else
			{
				if ( heroMoveFlags & MoveFlags_MirroredAnimation )
				{
					animation = &Tiles::getHeroRunMirroredAnimation();
				} else
				{
					animation = &Tiles::getHeroRunAnimation();
				}
			}

			const int frame = int( totalTime * settings.gameplay.heroAnimationFps ) % animation->size();
			DrawTexturePro( tiles.getTexture(), tiles.getRectangleForTile( animation->at( frame ) ), Rectangle{ heroPosition.x, heroPosition.y, 1, 1 }, Vector2{ 0, 0 }, 0, WHITE );
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
			enemy.progress = fmod( enemy.progress, fullLength );

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

struct BestTime
{
	int level;
	float time;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(BestTime, level, time);
};

struct Savegame
{
	std::vector<BestTime> bestTimes;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Savegame, bestTimes);
};

class GameFlow
{
public:
	const Tiles& tiles;
	const Settings& settings;
	ImFont* uiFont;
	bool screenChanged = false;
	bool shutdownRequested = false;
	Translator translator;

	GameFlow( const Tiles& _tiles, const Settings& _settings, ImFont* _uiFont ) : tiles( _tiles ), settings( _settings ), uiFont( _uiFont )
	{
		currentHandler = &GameFlow::splashScreen;

		translator.setLanguage( Language::English );
	}

	void step()
	{
		screenChanged = false;
		currentHandler( this );
	}

private:
	function<void( GameFlow* )> currentHandler;

	static const int maxLevels = 10;

	unique_ptr<Level> level;
	unique_ptr<Session> session;
	int nextLevel = 0;
	optional<float> bestTime;

	void pushUiStyle()
	{
		ImGui::PushFont( uiFont );
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0, 0, 0, 1 ) );
		ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0, 0, 0, 0 ) );
	}

	void popUiStyle()
	{
		ImGui::PopStyleColor( 3 );
		ImGui::PopFont();
	}

	static std::filesystem::path getSavegamePath() {
#ifdef __WINDOWS
		const std::filesystem::path home = std::getenv("APPDATA");
#else
		const std::filesystem::path home = std::getenv("HOME");
#endif
		return home / ".robodaniel" / "savegame.json";
	}

	void saveBestTime( const int level, const float bestTime )
	{
		Savegame savegame;

		if (std::filesystem::exists(getSavegamePath())) {
			std::ifstream stream(getSavegamePath());
			nlohmann::json json;
			stream >> json;
			savegame = json;
		}

		bool present = false;
		for (BestTime& best_time : savegame.bestTimes) {
			if (best_time.level == level) {
				best_time.time = bestTime;
				present = true;
			}
		}

		if (!present) {
			savegame.bestTimes.push_back(BestTime{ level, bestTime });
		}

		{
			std::error_code ec;
			std::filesystem::create_directories(getSavegamePath().parent_path(), ec);

			std::ofstream stream(getSavegamePath());
			nlohmann::json json;
			json = savegame;
			stream << json;
		}
	}

	optional<float> loadBestTime( const int level )
	{
		if (std::filesystem::exists(getSavegamePath())) {
			Savegame savegame;

			std::ifstream stream(getSavegamePath());
			nlohmann::json json;
			stream >> json;
			savegame = json;

			for (BestTime& best_time : savegame.bestTimes) {
				if (best_time.level == level) {
					return best_time.time;
				}
			}
		}
		
		return std::nullopt;
	}

	void splashScreen()
	{
		pushUiStyle();

		ImGui::CenterWindowForText( "____Robodaniel____" );
		if ( ImGui::Begin( "Splash Screen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
		{
			ImGui::CenteredText( "Robodaniel" );
			if ( translator.getLanguage() == Language::English )
			{
				if ( ImGui::CenteredButton( "Italiano" ) )
				{
					translator.setLanguage( Language::Italian );
				}
			} else if ( translator.getLanguage() == Language::Italian )
			{
				if ( ImGui::CenteredButton( "English" ) )
				{
					translator.setLanguage( Language::English );
				}
			}
			if ( ImGui::CenteredButton( translator.translate( "Play" ) ) )
			{
				nextLevel = 0;
				screenChanged = true;
				currentHandler = &GameFlow::initSession;
			}
			if ( ImGui::CenteredButton( translator.translate( "Select Level" ) ) )
			{
				auto bestTimes = loadBestTimes();
				screenChanged = true;
				currentHandler = std::bind( &GameFlow::selectLevel, this, bestTimes );
			}
			if ( ImGui::CenteredButton( translator.translate( "Best Times" ) ) )
			{
				auto bestTimes = loadBestTimes();
				screenChanged = true;
				currentHandler = std::bind( &GameFlow::showBestTimes, this, bestTimes );
			}
			if ( ImGui::CenteredButton( translator.translate( "Credits" ) ) )
			{
				screenChanged = true;
				currentHandler = &GameFlow::credits;
			}
#ifdef __WINDOWS
			if ( ImGui::CenteredButton( translator.translate( "Quit" ) ) )
			{
				shutdownRequested = true;
			}
#endif
		}
		ImGui::End();
		popUiStyle();
	}

	void selectLevel( const array< optional<float>, maxLevels > bestTimes )
	{
		pushUiStyle();
		ImGui::CenterWindowForText( "___Level XX___" );
		if ( ImGui::Begin( "Select Level", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
		{
			for ( int i = 0; i < maxLevels; ++i )
			{
				char caption[ 32 ];
				sprintf( caption, translator.translate( "Level %2d" ).c_str(), i + 1 );

				if ( bestTimes.at( i ).has_value() || ( i == 0 ) || ( i > 0 && bestTimes.at( i - 1 ).has_value() ) )
				{
					if ( ImGui::CenteredButton( caption ) )
					{
						nextLevel = i;
						screenChanged = true;
						currentHandler = &GameFlow::initSession;
					}
				} else
				{
					ImGui::CenteredTextDisabled( caption );
				}
			}
			if ( ImGui::CenteredButton( translator.translate( "Back" ) ) )
			{
				screenChanged = true;
				currentHandler = &GameFlow::splashScreen;
			}
		}
		ImGui::End();
		popUiStyle();
	}

	void credits()
	{
		pushUiStyle();
		ImGui::CenterWindowForText( "Tile and cloud art by Kenney Vleugels" );
		if ( ImGui::Begin( "Best times", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
		{
			ImGui::CenteredText( "Robodaniel v" BUILD_VERSION );
			ImGui::CenteredText( "Copyright (c) 2022-2023 Cinghy Creations" );
			ImGui::CenteredText( "" );
			ImGui::CenteredText( "raylib by Ramon Santamaria" );
			ImGui::CenteredText( "ImGui by Omar Cornut" );
			ImGui::CenteredText( "JSON for Modern C++ by Niels Lohmann" );
			ImGui::CenteredText( "Tile and cloud art by Kenney Vleugels" );
			ImGui::CenteredText( "Robot art by GameArt2D.com" );
			ImGui::CenteredText( "ProggyTiny font by Tristan Grimmer" );
			if ( ImGui::CenteredButton( translator.translate( "Back" ) ) )
			{
				screenChanged = true;
				currentHandler = &GameFlow::splashScreen;
			}
		}
		ImGui::End();
		popUiStyle();
	}

	array< optional<float>, maxLevels > loadBestTimes()
	{
		array< optional<float>, maxLevels > bestTimes;
		for ( int i = 0; i < maxLevels; ++i )
		{
			bestTimes.at( i ) = loadBestTime( i );
		}

		return bestTimes;
	}

	void showBestTimes( const array< optional<float>, maxLevels > bestTimes )
	{
		pushUiStyle();
		ImGui::CenterWindowForText( "Level XX      XXX.XXX s" );
		if ( ImGui::Begin( "Best times", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
		{
			float total = 0;
			bool hasTotal = true;
			for ( int i = 0; i < maxLevels; ++i )
			{
				if ( bestTimes.at( i ).has_value() )
				{
					ImGui::Text( translator.translate( "Level %2d      %7.3f s" ).c_str(), i + 1, *bestTimes.at( i ) );
					total += *bestTimes.at( i );
				} else
				{
					ImGui::Text( translator.translate( "Level %2d      ------- s" ).c_str(), i + 1 );
					hasTotal = false;
				}
			}
			if ( hasTotal )
			{
				ImGui::Text( translator.translate( "Total         %7.3f s" ).c_str(), total );
			} else
			{
				ImGui::Text( translator.translate( "Total         ------- s" ).c_str() );
			}
			if ( ImGui::CenteredButton( translator.translate( "Back" ) ) )
			{
				screenChanged = true;
				currentHandler = &GameFlow::splashScreen;
			}
		}
		ImGui::End();
		popUiStyle();
	}

	void initSession()
	{
		filesystem::path levelPath = string( "level" ) + to_string( nextLevel ) + ".csv";
		level.reset( new Level( levelPath ) );
		session.reset( new Session( tiles, *level, settings ) );
		bestTime = loadBestTime( nextLevel );

		screenChanged = true;
		currentHandler = &GameFlow::play;
	}

	void play()
	{
		// Step session
		session->step();

		// Render UI
		{
			ImGui::SetNextWindowPos( ImVec2( 30, 30 ) );
			pushUiStyle();
			if ( ImGui::Begin( "HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
			{
				ImGui::Text( translator.translate( "Coins %2d/%2d" ).c_str(), session->collectedCoins, session->totalCoins );
				ImGui::Text( translator.translate( "Time %7.3f" ).c_str(), session->totalTime );
				if ( bestTime.has_value() )
				{
					ImGui::Text( translator.translate( "Best %7.3f" ).c_str(), *bestTime );
				}
				if ( ImGui::Button( translator.translate( "Back" ).c_str() ) )
				{
					screenChanged = true;
					currentHandler = &GameFlow::splashScreen;
				}
			}
			ImGui::End();
			popUiStyle();
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
			if ( !bestTime.has_value() || session->totalTime < *bestTime )
			{
				saveBestTime( nextLevel, session->totalTime );
			}
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

		pushUiStyle();
		ImGui::CenterWindowForText( "_Level completed in xx.xxx seconds!_" );
		if ( ImGui::Begin( "Session completed", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
		{
			ImGui::Text( translator.translate( "Level completed in %.3f seconds!" ).c_str(), session->totalTime );
			if ( !bestTime.has_value() || session->totalTime < *bestTime )
			{
				ImGui::CenteredText( translator.translate( "New best time!" ) );
			}
			if ( nextLevel + 1 < maxLevels )
			{
				if ( ImGui::CenteredButton( translator.translate( "Next Level" ) ) )
				{
					session.reset();
					level.reset();

					++nextLevel;
					screenChanged = true;
					currentHandler = &GameFlow::initSession;
				}
			}
			if ( ImGui::CenteredButton( translator.translate( "Retry" ) ) )
			{
				session.reset();
				level.reset();

				screenChanged = true;
				currentHandler = &GameFlow::initSession;
			}
			if ( ImGui::CenteredButton( translator.translate( "Main Menu" ) ) )
			{
				session.reset();
				level.reset();

				screenChanged = true;
				currentHandler = &GameFlow::splashScreen;
			}
		}
		ImGui::End();
		popUiStyle();
	}

	void sessionFailed()
	{
		BeginMode2D( settings.debug.enableDebugCamera ? settings.debug.debugCamera : session->gameplayCamera );
		session->render();
		EndMode2D();

		pushUiStyle();
		ImGui::CenterWindowForText( "__Level failed!__" );
		if ( ImGui::Begin( "Session failed", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings ) )
		{
			ImGui::CenteredText( translator.translate( "Level failed!" ) );
			if ( ImGui::CenteredButton( translator.translate( "Retry" ) ) )
			{
				session.reset();
				level.reset();

				screenChanged = true;
				currentHandler = &GameFlow::initSession;
			}
			if ( ImGui::CenteredButton( translator.translate( "Main Menu" ) ) )
			{
				screenChanged = true;
				currentHandler = &GameFlow::splashScreen;
			}
		}
		ImGui::End();
		popUiStyle();
	}
};

int main()
{
	InitWindow( 1280, 720, "Game" );
	SetWindowState( FLAG_WINDOW_RESIZABLE );

	rlImGuiSetup(true);
	ImFont* uiFont = nullptr;
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF( "DroidSans.ttf", 24.0f );
		uiFont = io.Fonts->AddFontFromFileTTF( "ProggyTiny.ttf", 48.0f );
		rlImGuiReloadFonts();
	}

	array<Texture2D, 3> cloudTextures;
	cloudTextures[ 0 ] = LoadTexture( "cloud1.png" );
	cloudTextures[ 1 ] = LoadTexture( "cloud2.png" );
	cloudTextures[ 2 ] = LoadTexture( "cloud3.png" );

	typedef tuple<int, int, int> Cloud;
	vector<Cloud> clouds;

	auto regenClouds = [&clouds, &cloudTextures]()
	{
		clouds.clear();
		for ( int i = 0; i < 30; ++i )
		{
			clouds.push_back( { rand() % GetScreenWidth(), rand() % GetScreenHeight(), rand() % cloudTextures.size() } );
		}
	};
	regenClouds();

	Tiles tiles( "tiles.png", 128 );
	Settings settings;
	GameFlow flow( tiles, settings, uiFont );
	bool showSettings = false;

	while ( !WindowShouldClose() )
	{
		rlImGuiBegin();

		if ( ImGui::BeginDevMenuBar() )
		{
			if ( ImGui::BeginMenu( "Game" ) )
			{
				ImGui::MenuItem( "Show Settings", nullptr, &showSettings );
				if ( ImGui::MenuItem( "Create blank savegame" ) )
				{
					ofstream stream( "storage.data" );
					array<int, 100> blanckData;
					std::fill( blanckData.begin(), blanckData.end(), 0 );
					blanckData.at( 0 ) = 1;
					stream.rdbuf()->sputn( reinterpret_cast<const char*>( blanckData.data() ), sizeof( int ) * blanckData.size() );
				}
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
					ImGui::DragFloat( "Hero steps per Second", &settings.gameplay.heroStepsPerSecond, 0.01f );
					ImGui::SliderFloat( "Hero Animation FPS", &settings.gameplay.heroAnimationFps, 0.1f, 60.0f );
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
			const Color bgColor{ 208, 244, 247, 255 };
			ClearBackground( bgColor );

			for ( const auto& cloud : clouds )
			{
				DrawTextureEx( cloudTextures.at( get<2>( cloud ) ), Vector2{ float( get<0>( cloud ) ), float( get<1>( cloud ) ) }, 0, 2, WHITE );
			}

			flow.step();

			if ( flow.screenChanged )
			{
				regenClouds();
			}

			if ( flow.shutdownRequested )
			{
				break;
			}

			rlImGuiEnd();
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
