cmake_minimum_required(VERSION 3.20)
project(KVStore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg toolchain
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# --- Библиотека kv ---
add_library(kv
    src/Storage.cpp
    src/Database.cpp
)
target_include_directories(kv PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# --- CLI-демо ---
add_executable(kv_demo apps/main.cpp)
target_link_libraries(kv_demo PRIVATE kv)

# --- Тесты ---
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    add_executable(kv_tests
        tests/test_storage.cpp
        tests/test_database.cpp
    )
    target_link_libraries(kv_tests PRIVATE kv GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(kv_tests)
endif()

# --- Бенчмарки ---
option(BUILD_BENCHMARKS "Build benchmarks" ON)
if(BUILD_BENCHMARKS)
    find_package(benchmark CONFIG REQUIRED)

    add_executable(kv_bench benchmarks/bench_database.cpp)
    target_link_libraries(kv_bench PRIVATE kv benchmark::benchmark)
endif()

# Все exe в bin/
set_target_properties(kv_demo kv_tests kv_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)