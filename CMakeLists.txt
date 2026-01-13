cmake_minimum_required(VERSION 4.1)
project(WhiteBoard)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimalizacie pre release
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Najdi raylib
find_package(raylib REQUIRED)

# Zdrojove subory
set(SOURCES
        main.cpp
        src/Canvas.cpp
        src/Palette.cpp
        src/Editor.cpp
)

# Hlavickove subory
set(HEADERS
        src/Canvas.h
        src/Palette.h
        src/Editor.h
)

# Vytvor spustitelny subor
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external
)

# Linkuj raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Compile features pre C++20
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
