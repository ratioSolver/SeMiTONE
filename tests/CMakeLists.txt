add_executable(sat_lib_tests test_sat.cpp)
target_link_libraries(sat_lib_tests PRIVATE SeMiTONE json)

add_executable(lra_lib_tests test_lra.cpp)
target_link_libraries(lra_lib_tests PRIVATE SeMiTONE json)

add_executable(ov_lib_tests test_ov.cpp)
target_link_libraries(ov_lib_tests PRIVATE SeMiTONE json)

add_executable(dl_lib_tests test_dl.cpp)
target_link_libraries(dl_lib_tests PRIVATE SeMiTONE json)

add_test(NAME SAT_LibTest COMMAND sat_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME LRA_LibTest COMMAND lra_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME OV_LibTest COMMAND ov_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME DL_LibTest COMMAND dl_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})