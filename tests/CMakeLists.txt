add_executable(sat_lib_tests test_sat.cpp)
target_include_directories(sat_lib_tests PRIVATE ../include ../extern/json/include)
target_link_libraries(sat_lib_tests PRIVATE SeMiTONE)

add_test(NAME SAT_LibTest COMMAND sat_lib_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})