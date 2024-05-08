cmake_minimum_required( VERSION 3.16 )
set( CMAKE_CXX_STANDARD 17 )
project( robodaniel VERSION 1.1.1 )

add_compile_definitions( $<$<CONFIG:Release>:__RELEASE> )
add_compile_definitions( BUILD_VERSION="${PROJECT_VERSION}" )

if (WIN32)
    add_compile_definitions( __WINDOWS )
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup" )
elseif (EMSCRIPTEN)
    add_compile_definitions( __HTML5 )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s ASYNCIFY" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/build@/ --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/etc/shell.html" )
    set( CMAKE_EXECUTABLE_SUFFIX ".html" )
endif ()

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

add_executable( robodaniel
	src/robodaniel/intmath.hpp
	src/robodaniel/main.cpp
)
target_include_directories( robodaniel PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/robodaniel" )
target_link_libraries( robodaniel PUBLIC ImGui rlImGui raylib nlohmann_json )
target_precompile_headers( robodaniel PUBLIC <raylib.h> <nlohmann/json.hpp> )

if (EMSCRIPTEN)
    set_target_properties( robodaniel PROPERTIES OUTPUT_NAME index )
endif()
