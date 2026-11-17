
cmake_minimum_required(VERSION 3.20)

project(minorb LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
add_executable(minorb
    main.cpp
    src/lexer/Lexer.cpp
)

target_include_directories(minorb PRIVATE
    src/lexer
)