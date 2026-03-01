# default cmake setup
cmake_minimum_required(VERSION 3.15)
# define the project
project(TINY_JOBS_PROJ LANGUAGES CXX)

# option to enable examples
option(TINY_JOBS_COMPILE_EXAMPLES "enable or disable if the examples will be build" OFF)
option(TINY_JOBS_SUPPRESS_BUILD_MSG "enable or disable the logging of stuff while tiny jobs is build" ON)

# add tiny jobs
add_subdirectory(TinyJobs)

# do build prep if examples should be compiled
if (TINY_JOBS_COMPILE_EXAMPLES)
    # Set Release as default build type if none is specified
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()

    # add the examples
    add_subdirectory(TinyJobs/examples)

endif()

# Required for documentation generations
find_package(Doxygen)

# Can only progress of doxygen was found
if(Doxygen_FOUND)
    # setup doxygen
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # configure the doxyfile
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # add the custom documentation target
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()