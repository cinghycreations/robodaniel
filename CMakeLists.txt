cmake_minimum_required( VERSION 3.16 )
project( game )
set( CMAKE_CXX_STANDARD 17 )

if (EMSCRIPTEN)
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s ASSERTIONS=1 -s ASYNCIFY" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 --preload-file D:/Projects/experiments/robodaniel/build@/" )
    set( CMAKE_EXECUTABLE_SUFFIX ".html" )
endif ()

add_subdirectory( raylib )
add_subdirectory( src/ImGui )
add_subdirectory( src/game )
