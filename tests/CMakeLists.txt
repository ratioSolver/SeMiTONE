add_executable(network_tests test_network.cpp)
add_dependencies(network_tests SeMiTONE)
target_link_libraries(network_tests PRIVATE SeMiTONE)
setup_sanitizers(network_tests)

add_test(NAME SeMiTONE_NetworkTest COMMAND network_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})