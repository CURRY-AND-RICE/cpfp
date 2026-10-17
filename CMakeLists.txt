cmake_minimum_required(VERSION 3.25)
project(cpfp VERSION 0.1.0 LANGUAGES CXX)

option(CPFP_STRICT_ATCODER "Require GCC 15.2.x / Boost 1.88.x (AtCoder)" ON)
option(CPFP_SANITIZERS "Enable AddressSanitizer and UBSan for test executables" OFF)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "cpfp supports GCC only")
endif()

if(CPFP_STRICT_ATCODER)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.2" OR
       NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.3")
        message(FATAL_ERROR "AtCoder target is GCC 15.2.x; found ${CMAKE_CXX_COMPILER_VERSION}. Set CPFP_STRICT_ATCODER=OFF only for local development.")
    endif()
endif()

include(cmake/cpfpFindBoost.cmake)
if(CPFP_STRICT_ATCODER)
    cpfp_find_boost_headers(_CPFP_BOOST_INCLUDE_DIR REQUIRE_1_88)
else()
    cpfp_find_boost_headers(_CPFP_BOOST_INCLUDE_DIR)
endif()

add_library(cpfp INTERFACE)
add_library(cpfp::cpfp ALIAS cpfp)
target_compile_features(cpfp INTERFACE cxx_std_23)
set_target_properties(cpfp PROPERTIES CXX_EXTENSIONS ON)
target_include_directories(cpfp INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>")
target_include_directories(cpfp SYSTEM INTERFACE
    "$<BUILD_INTERFACE:${_CPFP_BOOST_INCLUDE_DIR}>")

include(CTest)
if(BUILD_TESTING)
    function(cpfp_test_target name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE cpfp::cpfp)
        target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic -Werror)
        if(CPFP_SANITIZERS)
            target_compile_options(${name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${name} PRIVATE -fsanitize=address,undefined)
        endif()
        add_test(NAME ${name} COMMAND ${name})
    endfunction()

    cpfp_test_target(cpfp_smoke tests/smoke.cpp)
    cpfp_test_target(cpfp_headers tests/header_self_contained.cpp)

    # Each public header must compile independently, not only via the umbrella.
    foreach(header IN ITEMS arithmetic combinator comparison fold logical experimental)
        set(source "${CMAKE_CURRENT_BINARY_DIR}/header_${header}.cpp")
        file(WRITE "${source}" "#include \"cpfp/${header}.hpp\"\nint main() { return 0; }\n")
        cpfp_test_target(cpfp_header_${header} "${source}")
    endforeach()
    add_executable(cpfp_ex1 examples/ex1.cpp)
    target_link_libraries(cpfp_ex1 PRIVATE cpfp::cpfp)

    foreach(failure IN ITEMS too_many_args too_many_args_staged)
        add_test(NAME rejected_${failure}
            COMMAND "${CMAKE_COMMAND}"
                "-DCXX=${CMAKE_CXX_COMPILER}"
                "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/negative/${failure}.cpp"
                "-DCPFP_INCLUDE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/include"
                "-DCPFP_BOOST_DIR=${_CPFP_BOOST_INCLUDE_DIR}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_failure.cmake")
    endforeach()
endif()

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS cpfp EXPORT cpfpTargets)
install(EXPORT cpfpTargets NAMESPACE cpfp:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpfp")

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpfpConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cpfpConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpfp")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cpfpConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cpfpConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cpfpConfigVersion.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpfpFindBoost.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpfp")
