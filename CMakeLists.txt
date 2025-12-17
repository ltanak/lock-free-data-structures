# cmake project setup
cmake_minimum_required(VERSION 3.23)


project(lock_free_data_structures LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# .cpp files under src/
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Remove python benchmarking sources explicitly
list(FILTER PROJECT_SOURCES EXCLUDE REGEX
    ".*/benchmarking/python/.*"
)

set(PROJECT_LIB_SOURCES ${PROJECT_SOURCES})
list(FILTER PROJECT_LIB_SOURCES EXCLUDE REGEX ".*/main.cpp")

add_executable(lock_free_data_structures ${PROJECT_SOURCES})

target_include_directories(lock_free_data_structures
PRIVATE
${CMAKE_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)

# google test
include(FetchContent)
FetchContent_Declare(
    googletest
    # Specify the commit you depend on and update it regularly
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Now simply link against gtest or gtest_main as needed. Eg
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/gtests/*.cpp
    )
    
    add_executable(unit_tests ${TEST_SOURCES} ${PROJECT_LIB_SOURCES})
    
    target_include_directories(unit_tests
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    )
    
    # link projects together
    target_link_libraries(unit_tests
    PRIVATE
    gtest_main
    Threads::Threads
    )
    
    target_compile_definitions(unit_tests
        PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )
    
    target_compile_definitions(lock_free_data_structures
        PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )
    
    enable_testing()
    add_test(NAME unit_tests COMMAND unit_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})