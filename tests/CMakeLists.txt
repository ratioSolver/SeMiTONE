add_executable(sat_tests test_sat.cpp)
add_dependencies(sat_tests SeMiTONE json)
target_link_libraries(sat_tests PRIVATE SeMiTONE json)

add_executable(lra_tests test_lra.cpp)
add_dependencies(lra_tests SeMiTONE json)
target_link_libraries(lra_tests PRIVATE SeMiTONE json)

add_executable(ov_tests test_ov.cpp)
add_dependencies(ov_tests SeMiTONE json)
target_link_libraries(ov_tests PRIVATE SeMiTONE json)

add_executable(rdl_tests test_rdl.cpp)
add_dependencies(rdl_tests SeMiTONE json)
target_link_libraries(rdl_tests PRIVATE SeMiTONE json)

add_test(NAME SAT_Test COMMAND sat_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME LRA_Test COMMAND lra_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME RDL_Test COMMAND rdl_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})