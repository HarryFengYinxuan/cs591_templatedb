add_executable(simple_benchmark simple_benchmark.cpp)
target_link_libraries(simple_benchmark PUBLIC templatedb
     ${Boost_LIBRARIES})

add_executable(bloom_test bloom_test.cpp)

# target_link_libraries(bloom_test Boost::filesystem)
target_link_libraries(bloom_test PUBLIC BF ${Boost_LIBRARIES})
