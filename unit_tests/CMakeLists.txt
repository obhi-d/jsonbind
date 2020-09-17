
project(cpphash_general_tests)

include(ExternalProject)

ExternalProject_Add(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/Catch2
        STEP_TARGETS install
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/out/external/install
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
        -DCATCH_BUILD_TESTING=OFF
        )




## Basic tests
add_executable(jsbind-validity
      src/main.cpp)
add_test(jsbind-validity jsbind-validity)
add_dependencies(jsbind-validity Catch2-install)
add_dependencies(jsbind-validity jsonbind::jsonbind)
target_include_directories(jsbind-validity PRIVATE "${CMAKE_SOURCE_DIR}/include")
