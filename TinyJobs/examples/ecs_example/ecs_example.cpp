/**
 * @file ecs_example.cpp
 * @author DM8AT
 * @brief a file showing how the tiny job system works in an ecs style workload
 * 
 * This file simulates an optimized ECS workload and shows the time difference of solving the N-Body problem using
 * one time the tiny job multithreading system and the other time a naive, single threaded implementation. 
 * 
 * @version 1.0
 * @date 2026-02-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */
//used for text input / output
#include <iostream>

//include the library
#include "TinyJobs.h"

/**
 * @brief a simple utility to time how long a scope takes
 */
class ScopeTimer {
public:
    /**
     * @brief select a specific clock
     */
    using clock = std::chrono::steady_clock;

    /**
     * @brief Construct a new Scope Timer
     * 
     * @param name the name of the scope
     */
    ScopeTimer(std::string_view name = "")
     : m_name(name), m_start(clock::now()) 
    {}

    /**
     * @brief Destroy the Scope Timer
     * 
     * This finishes the timing and prints it
     */
    ~ScopeTimer() {
        //compute the duration in milliseconds as a double
        const auto end = clock::now();
        const auto dur = std::chrono::duration<double, std::milli>(end - m_start);
        //if the name exists, print with the name, else just the duration
        if (m_name.empty()) {
            std::cout << dur.count() << " ms\n";
        } else {
            std::cout << m_name << ": " << dur.count() << " ms\n";
        }
    }

private:
    /**
     * @brief store the name of the scope
     */
    std::string_view m_name;
    /**
     * @brief store the starting point of the timer
     */
    clock::time_point m_start;
};

/**
 * @brief a structure that stores a position
 * 
 * Also it has a few additional utility functions
 */
struct Position { 
    float x,y,z; 

    //Addition
    Position operator+(const Position& other) const 
    {return { x + other.x, y + other.y, z + other.z };}

    //Subtraction
    Position operator-(const Position& other) const 
    {return { x - other.x, y - other.y, z - other.z };}

    //Scalar multiplication
    Position operator*(float scalar) const 
    {return { x * scalar, y * scalar, z * scalar };}

    //Scalar division
    Position operator/(float scalar) const 
    {return { x / scalar, y / scalar, z / scalar };}
};
/**
 * @brief a structure that stores a position
 */
struct Velocity {float x,y,z;};
/**
 * @brief a structure that stores a mass
 */
struct Mass {float m;};

//amount of test objects
static constexpr size_t N = 1E5;
//the amount of entities per chunk
static constexpr size_t CHUNK_SIZE = 1024;

//gravitational constant
static constexpr float G = 6.67430E-11f;

/**
 * @brief a function to simulate the gravitational pull between objects on a single chunk of entities
 * 
 * This simulates the N-Body problem. It has a time complexity of O(N^2), making it good for testing CPU and memory heavy loads. 
 * 
 * @param chunk_idx the index of the chunk to operate on
 * @param pos the position vector
 * @param vel the velocity vector
 * @param mass the mass vector
 */
void tick(size_t chunk_idx, std::vector<Position>& pos, std::vector<Velocity>& vel, std::vector<Mass>& mass) {
    //start element index
    size_t i = chunk_idx * CHUNK_SIZE;
    //iterate over all elements
    //for each element: 
    //    Check if it is valid (if not, stop)
    //    Compute the acceleration using accurate gravity simulations
    //    Update the velocity using the acceleration
    //    Update the position using the velocity
    for (size_t j = 0; j < CHUNK_SIZE; ++j) {
        size_t idx = i+j;
        if (idx >= pos.size()) break;

        Position acc{0,0,0};
        for (size_t k = 0; k < N; ++k) {
            if (idx == k) continue;
            Position dir = pos[k] - pos[idx];
            float dist2 = dir.x*dir.x + dir.y*dir.y + dir.z*dir.z + 1e-5f;
            float invDist = 1.0f / sqrt(dist2);
            float f = G * mass[k].m * invDist*invDist;
            acc.x += dir.x * f;
            acc.y += dir.y * f;
            acc.z += dir.z * f;
        }

        pos[idx].x += (vel[idx].x += acc.x);
        pos[idx].y += (vel[idx].y += acc.y);
        pos[idx].z += (vel[idx].z += acc.z);
    }
}

int main() {
    //the main class of the Tiny Job system
    //this class acts as a scheduler and owns the thread pool
    Tiny::Jobs::Employer e;

    //store the ECS simulation columns
    std::vector<Position> pos(N);
    std::vector<Velocity> vel(N);
    std::vector<Mass> mass(N);
    //initialize positions, velocities and masses
    for (size_t i = 0; i < N; ++i) {
        pos[i] = {float(i%1000), float((i/1000)%1000), float(i/1000000)};
        vel[i] = {0.f, 0.f, 0.f};
        mass[i] = {1.f + (i % 10)};
    }

    //test for multi threaded implementation
    {
    //scope timer used to time the multi threaded time
    ScopeTimer s("Multi-Threaded");
    //initialize all tasks
    //the static version is used because it eliminates 3 new calls
    Tiny::Jobs::StaticBulkTask<(N + (CHUNK_SIZE-1)) / CHUNK_SIZE> tasks(tick, std::ref(pos), std::ref(vel), std::ref(mass));
    //add all the tasks in a single batch to the job system
    e.add_bulk(tasks);
    //lastly, wait for the system to return to idle
    //else, only the adding time would be measured, not the full execution time
    while (!e.idle())
    {std::this_thread::sleep_for(std::chrono::microseconds(1));}
    }

    //reset the values for the single-threaded test
    for (auto& p : pos) 
    {p = {0,0,0};}
    for (auto& v : vel) 
    {v = {0.5,1,1.5};}

    //test for single threaded implementation
    {
    //scope timer used to time the single threaded time
    ScopeTimer s("Single-Threaded");
    //naively iterate over all chunks and use the tick function
    //to update the position using gravitational attraction between the entities
    for (size_t i = 0; i < N/CHUNK_SIZE; ++i) 
    {tick(i, pos, vel, mass);}
    }
}