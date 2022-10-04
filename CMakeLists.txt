cmake_minimum_required(VERSION 3.0.0)
project(SeMiTONE VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(CTest)
enable_testing()

add_subdirectory(extern/json)

option(VERBOSE_LOG "Makes log more verbose" OFF)

file(GLOB SMT_SOURCES src/*.cpp src/arith/*.cpp src/arith/lra/*.cpp src/arith/dl/*.cpp src/ov/*.cpp)
file(GLOB SMT_HEADERS include/*.h include/arith/lra/*.h include/arith/*.h include/arith/dl/*.h include/ov/*.h include/utils/*.h)

add_library(${PROJECT_NAME} SHARED ${SMT_SOURCES})
GENERATE_EXPORT_HEADER(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include/arith $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include/arith/lra $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include/arith/dl $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include/ov $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include/utils $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_link_libraries(${PROJECT_NAME} PRIVATE json)

message(STATUS "Verbose log:            ${VERBOSE_LOG}")
if(VERBOSE_LOG)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VERBOSE_LOG)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    FILES ${SMT_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/semitone_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
configure_package_config_file(src/${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} PATH_VARS CMAKE_INSTALL_INCLUDEDIR)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake VERSION 1.0.0 COMPATIBILITY SameMajorVersion)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
