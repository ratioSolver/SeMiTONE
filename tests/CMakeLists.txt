add_executable(expr_lib_tests test_expr.cpp)
add_dependencies(expr_lib_tests SeMiTONE)
target_link_libraries(expr_lib_tests PRIVATE SeMiTONE)
setup_sanitizers(expr_lib_tests)

add_test(NAME SeMiTONE_ExprTest COMMAND expr_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})