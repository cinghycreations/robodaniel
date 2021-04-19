cmake_minimum_required( VERSION 3.16 )
project( game )
set( CMAKE_CXX_STANDARD 17 )

add_subdirectory( raylib )
add_subdirectory( src/ImGui )
add_subdirectory( src/game )
