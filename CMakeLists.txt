cmake_minimum_required(VERSION 3.21)
project(SeMiTONE VERSION 0.2.0 LANGUAGES CXX)

include(CTest)
enable_testing()

add_library(SeMiTONE src/solver.cpp src/context.cpp src/term.cpp)
target_compile_features(SeMiTONE PUBLIC cxx_std_17)
target_include_directories(SeMiTONE PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
if(NOT TARGET json)
    add_subdirectory(extern/json)
endif()
add_dependencies(SeMiTONE json)
target_link_libraries(SeMiTONE PUBLIC json)
setup_sanitizers(SeMiTONE)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME SeMiTONE)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
