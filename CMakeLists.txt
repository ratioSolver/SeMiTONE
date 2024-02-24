cmake_minimum_required(VERSION 3.5.0)
project(SeMiTONE VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")

include(CTest)
enable_testing()

add_subdirectory(extern/json)

add_library(${PROJECT_NAME} src/sat_core.cpp src/constr.cpp src/clause.cpp src/sat_eq.cpp src/sat_conj.cpp src/sat_disj.cpp src/theory.cpp src/ov/ov_theory.cpp src/ov/ov_eq.cpp src/lra/lra_theory.cpp src/dl/idl_theory.cpp src/dl/rdl_theory.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ov> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lra> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/dl>)
target_link_libraries(${PROJECT_NAME} PUBLIC json)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    if (ENABLE_COVERAGE)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
            target_link_libraries(${PROJECT_NAME} PUBLIC gcov)
        endif()
    endif()
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
