cmake_minimum_required( VERSION 3.16 )

add_executable( robodaniel
	cimgui_impl_raylib.cpp
	cimgui_impl_raylib.hpp
	intmath.hpp
	main.cpp
	droidsans.cpp
	proggytiny.cpp
)

target_include_directories( robodaniel PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" )
target_link_libraries( robodaniel PUBLIC ImGui raylib )
