cmake_minimum_required(VERSION 3.12)
project(templatedb VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})
# target_link_libraries(foo 
#   ${Boost_FILESYSTEM_LIBRARY}
# )

set(templatedb_lib
    src/templatedb/db.cpp
    src/templatedb/operation.cpp
    src/templatedb/bplustree.cpp
)

set(bloomfilter_lib
    src/BloomFilter/BloomFilter.cpp
    src/BloomFilter/murmurhash.cpp
)

add_library(${PROJECT_NAME} ${templatedb_lib})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

add_library(BF ${bloomfilter_lib})
target_include_directories(BF PUBLIC ${PROJECT_SOURCE_DIR}/src)

include_directories(external)

configure_file(external/CMakeLists.txt.in external/googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/googletest-download"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/googletest-download"
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/external/googletest-src"
                 "${CMAKE_BINARY_DIR}/external/googletest-build"
)

enable_testing()

add_subdirectory(examples)
add_subdirectory(tests)

