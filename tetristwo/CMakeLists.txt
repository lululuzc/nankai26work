cmake_minimum_required(VERSION 3.16)
project(Tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(tetris
    main.cpp
    menuscreen.h
    menuscreen.cpp
    dualgame.h
    dualgame.cpp
    tetrisboard.h
    tetrisboard.cpp
    tetriswindow.h
    tetriswindow.cpp
)

target_link_libraries(tetris PRIVATE Qt6::Widgets)
