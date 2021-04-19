add_executable( game
	cimgui_impl_raylib.cpp
	cimgui_impl_raylib.hpp
	intmath.hpp
	main.cpp
	rlgl.h )

target_include_directories( game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../raylib/include" )
target_link_libraries( game PUBLIC ImGui "${CMAKE_CURRENT_SOURCE_DIR}/../../raylib/lib/raylib.lib" winmm )
target_precompile_headers( game PUBLIC <raylib.h> <raymath.h> )
