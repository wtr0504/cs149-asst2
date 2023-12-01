cmake_minimum_required(VERSION 3.23)
project(asst2)

set(CMAKE_CXX_STANDARD 14)

include_directories(common)
include_directories(part_a)
include_directories(part_b)
include_directories(tests)

add_executable(asst2
        common/CycleTimer.h
        common/ppm.cpp
        part_a/itasksys.h
        part_a/tasksys.cpp
        part_a/tasksys.h
        part_b/itasksys.h
        part_b/tasksys.cpp
        part_b/tasksys.h
        tests/main.cpp
        tests/main_ref.cpp
        tests/tests.h
        tutorial/tutorial.cpp)
