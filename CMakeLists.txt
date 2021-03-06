cmake_minimum_required(VERSION 3.1)

project(jsonbind VERSION 0.0.1 LANGUAGES CXX)


option(JSONBIND_BUILD_TESTS "Build the unit tests when BUILD_TESTING is enabled." ON)
option(JSONBIND_BOOST_ALREADY_INCLUDED "Indicate if boost is already included."  OFF)
##
## CONFIGURATION
##
set(JSONBIND_TARGET_NAME         ${PROJECT_NAME})
set(JSONBIND_INCLUDE_BUILD_DIR   "${PROJECT_SOURCE_DIR}/include")


##
## TARGET
##
message("Target name: ${JSONBIND_TARGET_NAME}")
add_library(${JSONBIND_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${JSONBIND_TARGET_NAME} ALIAS ${JSONBIND_TARGET_NAME})
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
set(CMAKE_CXX_STANDARD 20)

target_include_directories(
    ${JSONBIND_TARGET_NAME}
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(
    ${JSONBIND_TARGET_NAME}
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/external>
)

if(NOT JSONBIND_BOOST_ALREADY_INCLUDED)

    include(ExternalProject)
    ExternalProject_Add(mp11
            GIT_REPOSITORY https://github.com/boostorg/mp11.git
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/mp11
            STEP_TARGETS install
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/out/external/install
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
            )

    target_include_directories(jsonbind INTERFACE "${CMAKE_SOURCE_DIR}/out/external/install/include")
    add_dependencies(jsonbind mp11)

endif()
##
## TESTS
##
include(CTest) #adds option BUILD_TESTING (default ON)

if(BUILD_TESTING AND JSONBIND_BUILD_TESTS)
    enable_testing()
    add_subdirectory(unit_tests)
endif()

##
## INSTALL
##
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

if(DEFINED CMAKE_INSTALL_DATAROOTDIR)
    configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)


    install(EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

    install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
endif()

install(DIRECTORY include DESTINATION .)
