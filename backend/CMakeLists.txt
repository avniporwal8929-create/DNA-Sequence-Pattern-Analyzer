cmake_minimum_required(VERSION 3.14)
project(DNAPatternAnalyzer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Core library
set(CORE_SOURCES
    src/DNAValidator.cpp
    src/Automaton.cpp
    src/NFA.cpp
    src/DFA.cpp
    src/RegexParser.cpp
    src/ThompsonConstruction.cpp
    src/SubsetConstruction.cpp
    src/PatternDetector.cpp
    src/SequenceComparator.cpp
)

add_library(DNACore STATIC ${CORE_SOURCES})

# Windows socket libraries
if(WIN32)
    set(SYSTEM_LIBS ws2_32 crypt32)
else()
    set(SYSTEM_LIBS pthread)
endif()

# Main backend server executable
add_executable(dna_server src/main.cpp)
target_link_libraries(dna_server PRIVATE DNACore ${SYSTEM_LIBS})

# Unit tests executable
add_executable(unit_tests backend/tests/run_tests.cpp)
target_link_libraries(unit_tests PRIVATE DNACore ${SYSTEM_LIBS})

# Individual tests
add_executable(test_dna_validator tests/test_dna_validator.cpp)
target_link_libraries(test_dna_validator PRIVATE DNACore)

add_executable(test_regex tests/test_regex.cpp)
target_link_libraries(test_regex PRIVATE DNACore)

add_executable(test_nfa tests/test_nfa.cpp)
target_link_libraries(test_nfa PRIVATE DNACore)

add_executable(test_subset_construction tests/test_subset_construction.cpp)
target_link_libraries(test_subset_construction PRIVATE DNACore)

add_executable(test_pattern_detector tests/test_pattern_detector.cpp)
target_link_libraries(test_pattern_detector PRIVATE DNACore)

add_executable(test_sequence_comparator tests/test_sequence_comparator.cpp)
target_link_libraries(test_sequence_comparator PRIVATE DNACore)

enable_testing()
add_test(NAME AllUnitTests COMMAND unit_tests)
