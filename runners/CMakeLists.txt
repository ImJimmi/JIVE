if (JIVE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarking)
endif ()

if (JIVE_BUILD_DEMO_RUNNER)
    add_subdirectory(demo-runner)
endif ()

if (JIVE_BUILD_TEST_RUNNER)
    add_subdirectory(test-runner)
endif ()
