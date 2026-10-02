cmake_minimum_required(VERSION 3.14)
project(LogLens VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(loglens_core
  src/Parser.cpp
)
target_include_directories(loglens_core PUBLIC include)
target_compile_options(loglens_core PRIVATE -Wall -Wextra)

add_executable(loglens src/main.cpp)
target_link_libraries(loglens PRIVATE loglens_core)