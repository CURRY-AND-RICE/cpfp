function(cpfp_find_boost_headers result_variable)
    set(options REQUIRE_1_88)
    cmake_parse_arguments(CPFP_BOOST "${options}" "" "" ${ARGN})

    set(CPFP_BOOST_ROOT "" CACHE PATH "Root directory of the Boost distribution")
    if(CPFP_BOOST_ROOT)
        set(boost_root "${CPFP_BOOST_ROOT}")
    elseif(DEFINED ENV{BOOST_ROOT} AND NOT "$ENV{BOOST_ROOT}" STREQUAL "")
        set(boost_root "$ENV{BOOST_ROOT}")
    else()
        message(FATAL_ERROR
            "Set CPFP_BOOST_ROOT or the BOOST_ROOT environment variable to the Boost root directory")
    endif()

    find_path(boost_include_dir NAMES boost/hof.hpp
        PATHS "${boost_root}" "${boost_root}/include"
        NO_DEFAULT_PATH
        NO_CACHE)
    if(NOT boost_include_dir)
        message(FATAL_ERROR "Boost.HOF headers not found under ${boost_root}")
    endif()
    if(NOT EXISTS "${boost_include_dir}/boost/lambda2.hpp")
        message(FATAL_ERROR "Boost.Lambda2 headers not found under ${boost_include_dir}")
    endif()
    if(CPFP_BOOST_REQUIRE_1_88)
        file(STRINGS "${boost_include_dir}/boost/version.hpp" boost_version_macro
            REGEX "^#define BOOST_VERSION [0-9]+")
        if(NOT boost_version_macro MATCHES "BOOST_VERSION 108800")
            message(FATAL_ERROR "cpfp requires Boost 1.88.0; found ${boost_version_macro}")
        endif()
    endif()

    set(${result_variable} "${boost_include_dir}" PARENT_SCOPE)
endfunction()
