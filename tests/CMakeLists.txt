add_executable(sat_lib_tests test_sat.cpp)
add_dependencies(sat_lib_tests SeMiTONE json)
target_link_libraries(sat_lib_tests PRIVATE SeMiTONE json)

add_test(NAME SAT_LibTest COMMAND sat_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})