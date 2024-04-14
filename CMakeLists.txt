cmake_minimum_required(VERSION 3.5.0)
project(SeMiTONE VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")

include(CTest)
enable_testing()

add_subdirectory(extern/json)

option(BUILD_LISTENERS "Builds the listeners" OFF)
option(ENABLE_VISUALIZATION "Enable visualization features" OFF)

add_library(SeMiTONE src/sat_core.cpp src/constr.cpp src/clause.cpp src/theory.cpp src/ov/ov_theory.cpp src/lra/lra_theory.cpp src/lra/lra_assertion.cpp src/lra/lra_eq.cpp src/dl/idl_theory.cpp src/dl/rdl_theory.cpp)
add_dependencies(SeMiTONE json)
target_compile_features(SeMiTONE PUBLIC cxx_std_17)
target_include_directories(SeMiTONE PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ov> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lra> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/dl>)
target_link_libraries(SeMiTONE PUBLIC json)

message(STATUS "Build listeners: ${BUILD_LISTENERS}")
if(BUILD_LISTENERS)
    target_compile_definitions(SeMiTONE PUBLIC BUILD_LISTENERS)
endif()

message(STATUS "Enable SeMiTONE visualization features: ${ENABLE_VISUALIZATION}")
if(ENABLE_VISUALIZATION)
    target_compile_definitions(SeMiTONE PUBLIC ENABLE_VISUALIZATION)
endif()

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
