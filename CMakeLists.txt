cmake_minimum_required(VERSION 3.7)
project(flatmap)
#set(CMAKE_CXX_COMPILER /usr/bin/clang++-4.0)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(benchmark)
set(CATCH_DIR ${CMAKE_CURRENT_BINARY_DIR}/catch)
if(NOT EXISTS ${CATCH_DIR}/catch.hpp)
  if (NOT EXISTS ${CATCH_DIR})
    make_directory(${CATCH_DIR})
  endif()
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp  ${CATCH_DIR}/catch.hpp
    STATUS
    status
    LOG
    log
  )
  list(GET status 0 status_code)
  list(GET status 1 status_string)

  if(NOT status_code EQUAL 0)
    message(FATAL_ERROR "error downloading catch: ${status_string}"
            "${log}")
  endif()
endif()

set(SANTIZE "-fsanitize=address,undefined")
set(TEST_FLAGS "${SANITIZE} -Weverything -Wno-padded -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Wno-weak-vtables")
set(TEST_SOURCE_FILES flatmap_test.cpp flatmap.hpp)
add_executable(flatmap_test ${TEST_SOURCE_FILES})
set_target_properties(flatmap_test
                      PROPERTIES
                      COMPILE_FLAGS ${TEST_FLAGS}
                      LINK_FLAGS ${TEST_FLAGS})

set(BENCH_FLAGS "-stdlib=libc++")
target_include_directories(flatmap_test PRIVATE ${CATCH_DIR})
set(BENCHMARK_SOURCE_FILES flatmap_benchmark.cpp flatmap.hpp)
add_executable(flatmap_benchmark ${BENCHMARK_SOURCE_FILES} )
target_link_libraries(flatmap_benchmark benchmark)
target_compile_options(flatmap_benchmark PUBLIC ${BENCHMARK_FLAGS})
