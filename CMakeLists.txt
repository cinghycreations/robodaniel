cmake_minimum_required( VERSION 3.16 )
project( game )
set( CMAKE_CXX_STANDARD 17 )

add_executable( game src/main.cpp )
target_include_directories( game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/include" )
target_link_libraries( game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/raylib/lib/raylib.lib" winmm )
target_precompile_headers( game PUBLIC <raylib.h> )
