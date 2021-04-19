cmake_minimum_required( VERSION 3.16 )

add_library( raylib INTERFACE )
target_include_directories( raylib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" )
target_link_libraries( raylib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib.lib" winmm )
target_precompile_headers( raylib INTERFACE <raylib.h> <raymath.h> )
