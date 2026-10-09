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

add_test(NAME benchmark_writes_results_in_working_directory
         COMMAND particle_life benchmark 16 1 benchmark_test_results.json)
add_test(NAME benchmark_rejects_results_outside_working_directory
         COMMAND particle_life benchmark 16 1 ../benchmark_test_results.json)
set_tests_properties(benchmark_rejects_results_outside_working_directory PROPERTIES
                     PASS_REGULAR_EXPRESSION "must stay inside the working directory")
