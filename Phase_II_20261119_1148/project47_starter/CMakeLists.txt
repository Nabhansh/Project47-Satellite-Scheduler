cmake_minimum_required(VERSION 3.20)
project(Project47SatelliteScheduler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PROJECT47_ENABLE_STRICT_WARNINGS "Enable strict compiler warnings as errors" OFF)
option(PROJECT47_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(project47 INTERFACE)
target_include_directories(project47 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(PROJECT47_ENABLE_STRICT_WARNINGS)
        target_compile_options(project47 INTERFACE
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
    endif()
    if(PROJECT47_ENABLE_SANITIZERS)
        target_compile_options(project47 INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(project47 INTERFACE -fsanitize=address,undefined)
    endif()
endif()

add_executable(project47_cli src/main.cpp)
target_link_libraries(project47_cli PRIVATE project47)

add_executable(project47_tests tests/test_main.cpp)
target_link_libraries(project47_tests PRIVATE project47)

add_executable(project47_benchmarks benchmarks/benchmark_main.cpp)
target_link_libraries(project47_benchmarks PRIVATE project47)

enable_testing()
add_test(NAME project47_tests COMMAND project47_tests)
