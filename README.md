# TinyJobs

TinyJobs is a lightweight, header-only C++ job system with DAG-based scheduling and core-pinned worker threads.

## Features

 - Header-only, single-file library
 - Easy to integrate
 - DAG-based job execution
 - Requires C++ 23 (or higher)
 - Core pinned worker threads
     - Supported on Windows, Linux and Mac
 - Optional dependencies:
     - `moodycamel::ConcurrentQueue`
     - `boost::fiber` and `boost::context`
 - Can be compiled without external dependencies
     - Disabling lock-free queues replaces them with mutex-protected standard containers, which may significantly reduce throughput under contention.

## Getting Started

```cpp
#include <TinyJobs.h>

int main() {
    Tiny::Jobs::Employer employer;

    Tiny::Jobs::Job job;
    Tiny::Jobs::Task task = []{ /* work */ };
    job.add(task, 0);
    job.finalize();

    employer.add(&job);
    task.result().await();
}
```

### Installation

#### Header-only Integration

Copy `TinyJobs/include/TinyJobs.h` into your project and include it. If you want to use it without dependencies, define `TINY_JOBS_NO_FIBERS` and `TINY_JOBS_NO_LOCKFREE` before including `TinyJobs.h`. 

#### CMake integration

To integrate TinyJobs using CMake add `TinyJobs` as a subdirectory. 
```CMake
add_subdirectory(TinyJobs)
target_link_libraries(MyApp PRIVATE TinyJobs)
```
into your CMake building chain. 

### Building

#### Build Instructions

To build with the provided CMake file, you need to install `boost` on your system using a package provider. Boost won't be downloaded automatically, but Moodycamel will. 

#### CMake Example

An example CMake file could look like this:

```CMake
cmake_minimum_required(VERSION 3.15)
project(MY_PROJECT LANGUAGES CXX)

add_executable(MyApp main.cpp)
add_subdirectory(TinyJobs)
target_link_libraries(MyApp PRIVATE TinyJobs)
```

#### CMake Options

TinyJobs provides a few compile-time options that change the internal behavior as well as the required dependencies. They do not influence the user API. 

##### TINY_JOBS_NO_FIBERS

The CMake option `TINY_JOBS_NO_FIBERS` can be used to disable the fiber system directly during compilation. This will add the define `TINY_JOBS_NO_FIBERS` to the build to notify that fibers are disabled. When this option is enabled, Boost is no longer required and will not be added to the target. 

##### TINY_JOBS_NO_LOCKFREE

The CMake option `TINY_JOBS_NO_LOCKFREE` can be used to disable the lock-free implementation. This will add the define `TINY_JOBS_NO_LOCKFREE` to the build to notify that the lock-free implementation was disabled. When this option is enabled, `moodycamel` is no longer required and will not be added to the target. If this option is enabled, the lock-free concurrent queue will be replaced with a mutex and a normal queue, both from the standard library. 

### Example Usage

#### First Integration

To integrate TinyJobs into your project, include the file `TinyJobs.h`. The corresponding include directory is automatically added when using the CMake file. 

Example:
```cpp
#include <TinyJobs.h>
```

This will add all structures provided by TinyJobs. 

#### Basic Job Execution

A job groups multiple tasks and expresses dependencies between them. Tasks will execute once all their dependencies have completed. 

```cpp
#include <TinyJobs.h>
#include <iostream>

int main() {
    Tiny::Jobs::Employer employer;

    Tiny::Jobs::Job job;

    Tiny::Jobs::Task task1 = [] {std::cout << "First\n";};
    Tiny::Jobs::Task task2 = [] {std::cout << "Second\n";};

    auto first = job.add(task1, 0);
    job.add(task2, 0, {first});

    job.finalize();

    employer.add(&job);
    employer.waitIdle();
}
```

**Requirements:**

 * The job must be finalized before scheduling. 
 * The `Employer` must be started (default behavior unless disabled in constructor).
 * The job object must outlive its execution. 

#### Independent Task Execution

For single, independent tasks where no dependency graph is required, a `Task` can be scheduled directly. This avoids the overhead of constructing a full job.

```cpp
#include <TinyJobs.h>
#include <iostream>

int main() {
    Tiny::Jobs::Employer employer;

    Tiny::Jobs::Task task([] {
        std::cout << "Hello World\n";
    });

    Tiny::Jobs::Job::TaskNode node = &task;
    employer.add(&node);

    task.result().await();
}
```

This approach is suitable for:
 * Fire-and-forget tasks
 * Small work items
 * Situations where no dependency tracking is required

#### Bulk Execution

To efficiently spawn many similar tasks, `BulkTask` or `StaticBulkTask` can be used. These distribute work across all worker threads with minimal scheduling overhead.

##### Compile-time sized bulk task

```cpp
#include <TinyJobs.h>
#include <iostream>

int main() {
    Tiny::Jobs::Employer employer;

    Tiny::Jobs::StaticBulkTask<10000> task(
        [](size_t i) {
            //Each invocation receives its index
        }
    );

    employer.add_bulk(task);
    employer.waitIdle();
}
```

##### Runtime-sized bulk task

```cpp
#include <TinyJobs.h>
#include <iostream>

int main() {
    Tiny::Jobs::Employer employer;

    Tiny::Jobs::BulkTask task(
        10000,
        [](size_t i) {
            //Work for index i
        }
    );

    employer.add_bulk(task);
    employer.waitIdle();
}
```

## Threading Model

TinyJobs uses a fixed-size worker pool managed by `Employer` instances. 

### Worker Threads

 * By default, the number of worker threads is determined automatically (0 = auto).
 * Threads are pinned to hardware cores.
 * An optional pinning offset allows multiple Employer instances to coexist without overlapping core assignments.


### Fiber Layout (Optional)

If enabled:

 * Each worker thread owns a configurable number of fibers.
 * Tasks may be cooperatively multiplexed within a thread.
 * Boost.Fiber and Boost.Context are required.

If disabled:

 * Tasks execute directly on worker threads.
 * No fiber dependency is required.


### Lifetime and Shutdown

 * `Employer` is designed to be long-lived.
 * Upon destruction, it blocks until all scheduled work has completed.
 * Jobs and tasks must remain valid until execution finishes.


### Dependency Handling

 * Tasks within a job execute only when all declared dependencies are satisfied.
 * Circular dependencies are not executed and may result in permanently stalled tasks.
 * Dependency limits may be enforced in debug builds.


## Documentation 

The library is fully documented using Doxygen. To generate a full documentation yourself, download the library and build the CMake target `doc`. This will generate the documentation at `TinyJobs/docs/html`. 