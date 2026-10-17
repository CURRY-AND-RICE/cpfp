foreach(REQUIRED CXX SOURCE CPFP_INCLUDE_DIR CPFP_BOOST_DIR)
    if(NOT DEFINED ${REQUIRED})
        message(FATAL_ERROR "Missing -D${REQUIRED}")
    endif()
endforeach()

execute_process(
    COMMAND "${CXX}" -std=gnu++23 -fsyntax-only
        "-I${CPFP_INCLUDE_DIR}" "-I${CPFP_BOOST_DIR}" "${SOURCE}"
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERRORS)

if(RESULT EQUAL 0)
    message(FATAL_ERROR "Compilation unexpectedly SUCCEEDED for ${SOURCE}")
endif()
if(NOT ERRORS MATCHES "error:")
    message(FATAL_ERROR "Compilation failed unexpectedly without compiler diagnostics: ${ERRORS}")
endif()
message(STATUS "As expected, GCC rejected ${SOURCE}")
