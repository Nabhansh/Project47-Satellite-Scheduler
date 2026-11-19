cmake_minimum_required(VERSION 3.20)
project(Project47SatelliteScheduler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(project47 INTERFACE)
target_include_directories(project47 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(project47_cli src/main.cpp)
target_link_libraries(project47_cli PRIVATE project47)

add_executable(project47_tests tests/test_main.cpp)
target_link_libraries(project47_tests PRIVATE project47)

enable_testing()
add_test(NAME project47_tests COMMAND project47_tests)
