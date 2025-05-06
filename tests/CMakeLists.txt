add_executable(test1 test.cpp)
target_include_directories(test1 PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(test1 PUBLIC chatservercpp)
add_test(NAME test1 COMMAND test1)

add_executable(test_client test_client.cpp)
target_include_directories(test_client PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(test_client PUBLIC chatservercpp)
add_test(NAME test_client COMMAND test_client)
