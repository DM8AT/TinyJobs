# library options
option(TINY_JOBS_NO_FIBERS "A function to disable the usage of fibers" OFF)
option(TINY_JOBS_NO_LOCKFREE "A function to disable the lockfree queue usage and replace it with stl-only queues (using a mutex for thread safety)" OFF)

# debug msg
if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
message(STATUS "[TinyJobs] Started configuration of main module")
endif()

# Tiny Jobs is header only, so create an interface library
add_library(TINY_JOBS INTERFACE)

# Add the include path
target_include_directories(TINY_JOBS INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# define a conditional debug flag
target_compile_definitions(TINY_JOBS INTERFACE
    $<$<CONFIG:Debug>:__TINY_ECS_DEBUG>
)

# check if lock-free is enabled
if (TINY_JOBS_NO_LOCKFREE)
    # notify that the lockfree implementation was disabled
    target_compile_definitions(TINY_JOBS INTERFACE TINY_JOBS_NO_LOCKFREE)

    # debug msg
    if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
    message(STATUS "[TinyJobs] Marked the lock-free queue system as disabled")
    endif()
else()
    # download moodycamel from git if it is not installed
    include(FetchContent)
    FetchContent_Declare(
    moodycamel_concurrentqueue
    GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
    GIT_TAG master
    )
    FetchContent_MakeAvailable(moodycamel_concurrentqueue)

    # Add include path
    target_include_directories(TINY_JOBS INTERFACE ${moodycamel_concurrentqueue_SOURCE_DIR})

    # debug msg
    if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
    message(STATUS "[TinyJobs] Lock-free system is enabled, added \"moodycamel\" for \"ConcurrentQueue\"")
    endif()
endif()

# check for fiber enabled
if (TINY_JOBS_NO_FIBERS)
    # notify that the fibers were disabled
    target_compile_definitions(TINY_JOBS INTERFACE TINY_JOBS_NO_FIBERS)

    # debug msg
    if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
    message(STATUS "[TinyJobs] Marked the fiber system as disabled")
    endif()
else()
    # explicitly disable the new boost policy warning
    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 NEW)
    endif()
    # add boost::fiber and boost::context
    find_package(Boost REQUIRED COMPONENTS context fiber)
    target_include_directories(TINY_JOBS INTERFACE ${Boost_INCLUDE_DIRS})
    target_link_libraries(TINY_JOBS INTERFACE Boost::context Boost::fiber)

    # debug msg
    if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
    message(STATUS "[TinyJobs] The fiber system is enabled, added \"boost::context\" and \"boost::fiber\" for the fiber system")
    endif()
endif()

# debug msg
if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
message(STATUS "[TinyJobs] Finished configuration of the main module")
endif()