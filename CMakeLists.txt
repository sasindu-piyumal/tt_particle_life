cmake_minimum_required(VERSION 3.16)
project(particle_life LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(particle_sim src/simulation.cpp)
target_include_directories(particle_sim PUBLIC src)
target_compile_options(particle_sim PRIVATE -Wall -Wextra -Wpedantic)

add_executable(particle_life src/main.cpp)
target_link_libraries(particle_life PRIVATE particle_sim)
target_compile_options(particle_life PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(particle_life_tests tests/simulation_test.cpp)
target_link_libraries(particle_life_tests PRIVATE particle_sim)
target_compile_options(particle_life_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME simulation COMMAND particle_life_tests)

add_executable(stream_protocol_tests tests/stream_protocol_test.cpp)
target_link_libraries(stream_protocol_tests PRIVATE particle_sim)
target_compile_options(stream_protocol_tests PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(stream_protocol_tests PRIVATE
    STREAM_BINARY_PATH="$<TARGET_FILE:particle_life>")
add_test(NAME stream_protocol COMMAND stream_protocol_tests)

add_test(NAME benchmark_results_file
         COMMAND ${CMAKE_COMMAND}
                 -DBENCHMARK=$<TARGET_FILE:particle_life>
                 -DRESULTS=${CMAKE_CURRENT_BINARY_DIR}/benchmark_test_results.json
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark_results_file.cmake)
