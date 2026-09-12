cmake_minimum_required(VERSION 3.20)
project(spsc_queue LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(spsc_queue INTERFACE)
target_include_directories(spsc_queue INTERFACE include)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(spsc_queue_test tests/spsc_queue_test.cc)
target_link_libraries(spsc_queue_test PRIVATE spsc_queue GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(spsc_queue_test)