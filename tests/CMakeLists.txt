add_executable(expr_tests test_expr.cpp)
add_dependencies(expr_tests SeMiTONE)
target_link_libraries(expr_tests PRIVATE SeMiTONE)
setup_sanitizers(expr_tests)

add_executable(solver_tests test_solver.cpp)
add_dependencies(solver_tests SeMiTONE)
target_link_libraries(solver_tests PRIVATE SeMiTONE)
setup_sanitizers(solver_tests)

add_test(NAME SeMiTONE_ExprTest COMMAND expr_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME SeMiTONE_SolverTest COMMAND solver_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})