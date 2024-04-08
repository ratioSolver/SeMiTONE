cmake_minimum_required(VERSION 3.5.0)
project(SeMiTONE VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")

include(CTest)
enable_testing()

add_subdirectory(extern/json)

add_library(SeMiTONE src/sat_core.cpp src/constr.cpp src/clause.cpp src/theory.cpp src/ov/ov_theory.cpp src/lra/lra_theory.cpp src/lra/lra_assertion.cpp src/lra/lra_eq.cpp src/dl/idl_theory.cpp src/dl/rdl_theory.cpp)
add_dependencies(SeMiTONE json)
target_compile_features(SeMiTONE PUBLIC cxx_std_17)
target_include_directories(SeMiTONE PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ov> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lra> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/dl>)
target_link_libraries(SeMiTONE PUBLIC json)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(MSVC)
    target_compile_options(SeMiTONE PRIVATE /W4)
else()
    target_compile_options(SeMiTONE PRIVATE -Wall -Wextra -Wpedantic)
    if (ENABLE_COVERAGE)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(SeMiTONE PRIVATE --coverage)
            target_link_libraries(SeMiTONE PUBLIC gcov)
        endif()
    endif()
endif()

set(CPACK_PROJECT_NAME SeMiTONE)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
