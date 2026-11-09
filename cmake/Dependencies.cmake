include_guard()

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # Statically link Gtest
set(VENDORS_DIR "Vendors")

FetchContent_Declare(
    google-test
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723 # release-1.17.0
    SYSTEM
)

FetchContent_Declare(
    google-benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.5
    SYSTEM
)

FetchContent_Declare(
        Falcon
        GIT_REPOSITORY https://github.com/alan-j316/falcon-game-math.git
        GIT_TAG d16224b0c8112c21a565fb84e5b0686358652176
        SYSTEM
)

FetchContent_Declare(
        fast-gltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf.git
        GIT_TAG v0.9.1
        SYSTEM
)

FetchContent_Declare(
        sdl3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG d9d5536704d585616d4db3c8ba3c4ff6fc2757e1
        SYSTEM
)

FetchContent_Declare(
        fast-float
        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
        GIT_TAG v8.2.10
        SYSTEM
)


set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) # Disables google benchmark from creating its own test suite

FetchContent_MakeAvailable(google-test)
FetchContent_MakeAvailable(google-benchmark)
FetchContent_MakeAvailable(Falcon)
FetchContent_MakeAvailable(sdl3)
FetchContent_MakeAvailable(fast-float)
FetchContent_MakeAvailable(fast-gltf)

# Group the projects into a single folder
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "Google Test")
set_target_properties(benchmark benchmark_main PROPERTIES FOLDER "Google Benchmark")
set_target_properties(SDL3-static SDL3_test PROPERTIES FOLDER "${VENDORS_DIR}/SDL3")