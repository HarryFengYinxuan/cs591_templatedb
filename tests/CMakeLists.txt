find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

add_executable(basic_test basic_test.cpp)

add_executable(persistence_test persistence_test.cpp)
# target_link_libraries(persistence_test Boost::filesystem)

target_link_libraries(basic_test PUBLIC templatedb gtest
     ${Boost_LIBRARIES})
target_link_libraries(persistence_test 
    PUBLIC templatedb gtest ${Boost_LIBRARIES})

add_test(NAME basic_test COMMAND ${CMAKE_BINARY_DIR}/tests/basic_test)
add_test(NAME persistence_test COMMAND ${CMAKE_BINARY_DIR}/tests/persistence_test)
