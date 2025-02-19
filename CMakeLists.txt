cmake_minimum_required(VERSION 3.21)
project(SeMiTONE VERSION 0.2.0 LANGUAGES CXX)

include(CTest)
enable_testing()

option(BUILD_LISTENERS "Builds SeMiTONE listeners" OFF)

add_library(SeMiTONE src/semitone.cpp src/theory.cpp src/la_theory.cpp src/dl_theory.cpp)
target_compile_features(SeMiTONE PUBLIC cxx_std_17)
target_include_directories(SeMiTONE PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
if(NOT TARGET json)
    add_subdirectory(extern/json)
endif()
add_dependencies(SeMiTONE json)
target_link_libraries(SeMiTONE PUBLIC json)
setup_sanitizers(SeMiTONE)

message(STATUS "Build SeMiTONE listeners: ${BUILD_LISTENERS}")
if(BUILD_LISTENERS)
    target_compile_definitions(SeMiTONE PUBLIC BUILD_LISTENERS)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME SeMiTONE)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
