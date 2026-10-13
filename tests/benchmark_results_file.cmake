# Runs the benchmark twice into the same results file and checks that each run
# leaves exactly one complete JSON document and no temporary file behind.
# Expects -DBENCHMARK=<particle_life binary> -DRESULTS=<results path>.

file(REMOVE "${RESULTS}" "${RESULTS}.tmp")

foreach(run 1 2)
    execute_process(
        COMMAND "${BENCHMARK}" benchmark 16 3 "${RESULTS}"
        RESULT_VARIABLE status
        OUTPUT_QUIET)
    if(NOT status EQUAL 0)
        message(FATAL_ERROR "benchmark run ${run} exited with ${status}")
    endif()
    if(EXISTS "${RESULTS}.tmp")
        message(FATAL_ERROR "benchmark run ${run} left ${RESULTS}.tmp behind")
    endif()
    file(READ "${RESULTS}" contents)
    if(NOT contents MATCHES "^{\"simulation_fps\":[^{}\n]+}\r?\n$")
        message(FATAL_ERROR "benchmark run ${run} wrote unexpected results: ${contents}")
    endif()
endforeach()

file(REMOVE "${RESULTS}")
