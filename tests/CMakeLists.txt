add_executable(expr_tests test_expr.cpp)
add_dependencies(expr_tests SeMiTONE)
target_link_libraries(expr_tests PRIVATE SeMiTONE)
setup_sanitizers(expr_tests)

add_executable(network_tests test_network.cpp)
add_dependencies(network_tests SeMiTONE)
target_link_libraries(network_tests PRIVATE SeMiTONE)
setup_sanitizers(network_tests)

add_test(NAME SeMiTONE_ExprTest COMMAND expr_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME SeMiTONE_NetworkTest COMMAND network_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})