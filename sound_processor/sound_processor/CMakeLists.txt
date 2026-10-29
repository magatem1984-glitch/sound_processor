cmake_minimum_required(VERSION 3.16)
project(sound_processor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sound_processor_core
    src/application.cpp
    src/args_parser.cpp
    src/cmd_line_args_to_pipeline_converter.cpp
    src/error.cpp
    src/filters.cpp
    src/pipeline.cpp
    src/std_filter_producers.cpp
    src/wav_file.cpp
    src/waveform.cpp
)

target_include_directories(sound_processor_core PUBLIC include)

target_compile_options(sound_processor_core PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(sound_processor
    src/main.cpp
)

target_link_libraries(sound_processor PRIVATE sound_processor_core)
target_compile_options(sound_processor PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(sound_processor_tests
    tests/sound_processor_tests.cpp
)

target_link_libraries(sound_processor_tests PRIVATE sound_processor_core)
target_compile_options(sound_processor_tests PRIVATE -Wall -Wextra -Werror -pedantic)

enable_testing()
add_test(NAME sound_processor_tests COMMAND sound_processor_tests)
