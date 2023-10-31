cmake_minimum_required(VERSION 3.21)

project(
        CppHighConcurrency
        VERSION 1.0
        DESCRIPTION "CPP High Concurrency"
        LANGUAGES CXX
)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=c++2a")
enable_testing()

file(GLOB_RECURSE srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
add_executable(cpp_high_concurrency ${srcs})
# target_sources(cpp_high_concurrency PUBLIC ${srcs})

target_link_libraries(cpp_high_concurrency GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(cpp_high_concurrency)