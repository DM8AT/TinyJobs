# debug msg
if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
message(STATUS "[TinyJobs] Started configuration of the \"ECS_EXAMPLE\" example executable")
endif()

# create an executable from ecs_example source
add_executable(ECS_EXAMPLE ecs_example.cpp)
# set the used C++ version to 23 (required)
set_target_properties(ECS_EXAMPLE PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
)

# add tiny jobs
target_link_libraries(ECS_EXAMPLE PRIVATE TINY_JOBS)

# compiler-specific release flags
if(MSVC)
    # MSVC (Visual Studio Compiler) release flags
    target_compile_options(ECS_EXAMPLE PRIVATE $<$<CONFIG:Release>:/O2 /GL /DNDEBUG>)
    target_link_options(ECS_EXAMPLE PRIVATE $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>)
else()
    # GCC / Clang Release flags
    target_compile_options(ECS_EXAMPLE PRIVATE $<$<CONFIG:Release>:
        -Ofast -march=native -flto -DNDEBUG
        -ffunction-sections -fdata-sections
    >)
    target_link_options(ECS_EXAMPLE PRIVATE $<$<CONFIG:Release>:
        -flto -Wl,--gc-sections
    >)
endif()
# add debug compile options
target_compile_options(ECS_EXAMPLE PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
)

# debug msg
if (NOT TINY_JOBS_SUPPRESS_BUILD_MSG)
message(STATUS "[TinyJobs] Finished configuration of the \"ECS_EXAMPLE\" example executable")
endif()