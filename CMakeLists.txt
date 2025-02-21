cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()


set(CPP_FILES
    src/main.cpp
    src/BMaths/algebraicEquationSolver.cpp
    src/BMaths/differentialEquationSolver.cpp
    src/BMaths/DAESolve.cpp
    src/BMaths/function.cpp
    src/component.cpp
    src/fileParser.cpp
)

add_executable(main ${CPP_FILES})

add_custom_target(run
    COMMAND main
    DEPENDS main
)



