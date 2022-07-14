add_executable(sat_lib_tests test_sat.cpp)
target_include_directories(sat_lib_tests PRIVATE ../include ../extern/json/include)
target_link_libraries(sat_lib_tests PRIVATE SeMiTONE)

add_executable(lra_lib_tests test_lra.cpp)
target_include_directories(lra_lib_tests PRIVATE ../include ../include/lra)
target_link_libraries(lra_lib_tests PRIVATE SeMiTONE)

add_executable(ov_lib_tests test_ov.cpp)
target_include_directories(ov_lib_tests PRIVATE ../include ../include/ov)
target_link_libraries(ov_lib_tests PRIVATE SeMiTONE)

add_executable(dl_lib_tests test_dl.cpp)
target_include_directories(dl_lib_tests PRIVATE ../include ../include/lra ../include/dl)
target_link_libraries(dl_lib_tests PRIVATE SeMiTONE)

add_test(NAME SAT_LibTest COMMAND sat_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME LRA_LibTest COMMAND lra_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME OV_LibTest COMMAND ov_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME DL_LibTest COMMAND dl_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})