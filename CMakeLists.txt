cmake_minimum_required( VERSION 3.16 )
set( CMAKE_CXX_STANDARD 17 )
project( robodaniel VERSION 1.1.1 )

add_compile_definitions( __RELEASE=$<IF:$<CONFIG:Release>,1,0> )
add_compile_definitions( __ANDROID=$<IF:$<PLATFORM_ID:Android>,1,0> )
add_compile_definitions( __LINUX=$<IF:$<PLATFORM_ID:Linux>,1,0> )
add_compile_definitions( __WEB=$<IF:$<PLATFORM_ID:Emscripten>,1,0> )
add_compile_definitions( __WINDOWS=$<IF:$<PLATFORM_ID:Windows>,1,0> )
add_compile_definitions( BUILD_VERSION="${PROJECT_VERSION}" )

add_subdirectory( ext/raylib EXCLUDE_FROM_ALL )
add_subdirectory( ext/json EXCLUDE_FROM_ALL )

add_library( ImGui STATIC
	ext/ImGui/imconfig.h
	ext/ImGui/imgui.cpp
	ext/ImGui/imgui.h
	ext/ImGui/imgui_demo.cpp
	ext/ImGui/imgui_draw.cpp
	ext/ImGui/imgui_internal.h
	ext/ImGui/imgui_tables.cpp
	ext/ImGui/imgui_widgets.cpp
	ext/ImGui/imstb_rectpack.h
	ext/ImGui/imstb_textedit.h
	ext/ImGui/imstb_truetype.h
)
target_include_directories( ImGui PUBLIC "${CMAKE_SOURCE_DIR}/ext/ImGui" )

add_library( rlImGui ext/rlImGui/rlImGui.h ext/rlImGui/rlImGui.cpp ext/rlImGui/rlImGuiColors.h )
target_link_libraries( rlImGui PUBLIC raylib ImGui )
target_include_directories( rlImGui PUBLIC "${CMAKE_SOURCE_DIR}/ext/rlImGui" )

add_executable( robodaniel src/robodaniel/intmath.hpp src/robodaniel/main.cpp )
target_include_directories( robodaniel PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/robodaniel" )
target_link_libraries( robodaniel PUBLIC ImGui rlImGui raylib nlohmann_json )
target_precompile_headers( robodaniel PUBLIC <raylib.h> <nlohmann/json.hpp> )

install( TARGETS robodaniel RUNTIME DESTINATION "." )
install( DIRECTORY "${CMAKE_SOURCE_DIR}/build/" DESTINATION "." )
include( CPack )
