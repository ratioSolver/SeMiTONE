cmake_minimum_required(VERSION 3.5.0)
project(SeMiTONE VERSION 0.2.0 LANGUAGES CXX)

include(CTest)
enable_testing()

add_subdirectory(extern/json)

add_library(SeMiTONE src/sat_core.cpp src/constr.cpp)
target_compile_features(SeMiTONE PUBLIC cxx_std_17)
target_include_directories(SeMiTONE PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(SeMiTONE PUBLIC json)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
