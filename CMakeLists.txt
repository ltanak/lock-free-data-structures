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

add_executable(lock_free_data_structures ${PROJECT_SOURCES})

target_include_directories(lock_free_data_structures
    PRIVATE
      ${CMAKE_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(lock_free_data_structures PRIVATE Threads::Threads)

