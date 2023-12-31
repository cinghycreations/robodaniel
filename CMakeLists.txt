cmake_minimum_required( VERSION 3.16 )
project( robodaniel )
set( CMAKE_CXX_STANDARD 17 )

add_compile_definitions( $<$<CONFIG:Release>:__RELEASE> )
add_compile_definitions( BUILD_VERSION="1.1.1" )

if (WIN32)
    add_compile_definitions( __WINDOWS )
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup" )
elseif (EMSCRIPTEN)
    add_compile_definitions( __HTML5 )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s ASYNCIFY" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/build@/ --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/etc/shell.html" )
    set( CMAKE_EXECUTABLE_SUFFIX ".html" )
endif ()

add_subdirectory( raylib )
add_subdirectory( src/ImGui )
add_subdirectory( src/robodaniel )

if (EMSCRIPTEN)
    set_target_properties( robodaniel PROPERTIES OUTPUT_NAME index )
endif()
